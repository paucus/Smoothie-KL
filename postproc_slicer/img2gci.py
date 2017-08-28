#! /usr/bin/python

#
# Kikai Labs Post Processor (c) by Kikai Labs
#
# Kikai Labs Post Processor is licensed under a
# Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
#
# You should have received a copy of the license along with this
# work.  If not, see <http://creativecommons.org/licenses/by-nc-sa/3.0/>.
#

import sys
from kl_img import *

if __name__ == "__main__":
	if (len(sys.argv) < 3):
		print "params: <img> <gci>"
		exit(128)
	bmp2gci(sys.argv[1], sys.argv[2])
