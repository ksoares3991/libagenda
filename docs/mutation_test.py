#!/usr/bin/env python3
"""
Mutation test of the library (used for the report, item 11).

Usage (from the project root):   python3 docs/mutation_test.py [src/file.c ...]

For each source file it creates "mutants": copies of the code with ONE small
change (== to !=, < to <=, && to ||, + to -, an error return replaced by
AGENDA_OK, ...). For each mutant it runs `make test`. If the tests fail (or hang,
which counts as a failure) the mutant is "killed": the tests noticed the defect.
A "survivor" is either an equivalent mutant (the change does not alter the
behaviour) or a gap in the tests, so every survivor has to be read by a person.

The work is done on a temporary copy; the project tree is never modified.
"""
import os, re, shutil, subprocess, sys, tempfile

SOURCES = ['src/timeslot.c', 'src/schedule.c', 'src/conflicts.c', 'src/freetime.c']
SWAPS = [('==', '!='), ('!=', '=='), ('<=', '<'), ('>=', '>'), ('&&', '||'), ('||', '&&')]


def mutants(lines):
    in_comment = False
    for i, line in enumerate(lines):
        s = line.strip()
        if in_comment:
            in_comment = '*/' not in s
            continue
        if s.startswith('/*'):
            in_comment = '*/' not in s
            continue
        if not s or s.startswith(('*', '//', '#')):
            continue
        has_string = '"' in line
        for a, b in SWAPS:
            for m in re.finditer(re.escape(a), line):
                yield i, m.start(), a, b
        for m in re.finditer(r'(?<![<>\-=!])<(?![<=])', line):
            yield i, m.start(), '<', '<='
        for m in re.finditer(r'(?<![<>\-=!])>(?![>=])', line):
            yield i, m.start(), '>', '>='
        if not has_string:
            for m in re.finditer(r'(?<![+\-=])\+(?![+=])', line):
                yield i, m.start(), '+', '-'
            for m in re.finditer(r'(?<![\-=>])-(?![\->=])', line):
                yield i, m.start(), '-', '+'
        m = re.search(r'return (AGENDA_ERR_\w+);', line)
        if m:
            yield i, m.start(), m.group(0), 'return AGENDA_OK;'


def main():
    files = sys.argv[1:] or SOURCES
    root = os.getcwd()
    work = os.path.join(tempfile.mkdtemp(prefix='mutation_'), 'p')
    shutil.copytree(root, work, ignore=shutil.ignore_patterns('.git', 'build', 'lib', 'bin'))
    os.chdir(work)
    total, survivors = 0, []
    for f in files:
        original = open(f).read()
        lines = original.split('\n')
        for i, col, a, b in mutants(lines):
            mutated = lines[:]
            mutated[i] = lines[i][:col] + b + lines[i][col + len(a):]
            open(f, 'w').write('\n'.join(mutated))
            try:
                r = subprocess.run(['timeout', '20', 'make', 'test'], capture_output=True, text=True)
                killed = r.returncode != 0
            except Exception:
                killed = True
            total += 1
            if not killed:
                survivors.append((f, i + 1, lines[i].strip(), mutated[i].strip()))
                print('SURVIVED %s:%d  %s   =>   %s' % survivors[-1], flush=True)
        open(f, 'w').write(original)
    print('MUTANTS %d, KILLED %d, SURVIVORS %d' % (total, total - len(survivors), len(survivors)))
    shutil.rmtree(os.path.dirname(work), ignore_errors=True)


if __name__ == '__main__':
    main()
