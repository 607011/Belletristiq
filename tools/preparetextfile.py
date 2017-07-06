#!/usr/bin/env python3

import sys
import re

with open(sys.argv[1], 'r') as textfile:
	text = ''
	in_para = False
	join_spaced = False
	for line in textfile:
		l = line.strip()
		n = len(l)
		if n == 0:
			text += '\n\n'
			join_spaced = False
		elif n < 40:
			continue
		else:
			if join_spaced:
				text += ' ' + l
			else:
				text = text[0:-1] + l
			join_spaced = not l.endswith('-')

print(text)
