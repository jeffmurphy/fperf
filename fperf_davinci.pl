#!/usr/local/bin/perl -w
#
# $Header$
#
# NAME
#   fperf_davince [fperf.dat file] [davinci output file]
#
# DESCRIPTION
#   extract call graph from fperf.dat file and write a davinci
#   graph file.
#
# AUTHOR
#   jeff murphy
#

use strict;

print "fperf_davinci v1.0 (c) 1999 Nickel City Software\n\n";

my $f = shift;
my $d = shift;

my ($id) = 0;

die "usage: $0 [fperf data file] [davinci out file]\n" unless defined($f);
die "usage: $0 [fperf data file] [davinci out file]\n" unless defined($d);
open(FD, $f) || die "can't open file $f: $!";

# search for beginning of "RT func stats" section.

while(<FD>) {
  chomp;
  last if /^SECTION-CG:$/;
}

my (@cg);

while(<FD>) {
	chomp;
	last if /^SECTION-(.*):$/;
	my (@a) = split(/\t/);
	my (%c);
	foreach (@a) {
		my (@b) = split(/=/);
		if($#b == 0) {
			$c{'type'} = $_;
		} else {
			$c{$b[0]} = $b[1];
		}
	}
	push @cg, \%c;
}

close(FD);

print "read ".@cg." data elements from CG section.\n";

# call graph elements:
#   type  = ROOT | CHILDOF
#   thr   = thread id
#   f     = function name
#   pf    = parent function name
#   a     = function start address
#   pa    = parent function start address
#   ts    = total seconds spent in function (and children)
#   calls = total number of calls to function
#   avg   = average time spent in function (and children)

print "\ncall graph:\n";

# sort the call graph by thrId (to group elements into the same
# graph)

# search for ROOT nodes and print them out and their children too

open(FO, "> $d") || die "can't open file $d: $!";

print FO "[\n";

my ($count) = 0;
my ($roots) = 0;
foreach (@cg) {
	$roots ++ if ($_->{'type'} eq "ROOT");
}

foreach (sort {$a->{'thr'} <=> $b->{'thr'}} @cg) {
	if($_->{'type'} eq "ROOT") {
		printNode($_, \@cg);
		print FO " ," if (--$roots);
		print FO "\n";
	} 
}

print FO "]\n";

exit 0;

sub printChildren {
	my $p = shift;
	my $cg = shift;

	# look for children of this parent and print them out

	foreach my $c (@$cg) {
		if(($c->{'type'} eq "CHILDOF") && 
		   ($c->{'pf'} eq $p->{'f'})   &&
		   ($c->{'thr'} eq $p->{'thr'}) &&
		   (!defined($c->{'dvid'}))
		  ) {
			my $o = "$c->{'f'} $c->{'thr'}";
			my $text = "$c->{'f'}\\n$c->{'avg'}";

			# print out an edge and stick us (the child)
			# at the end of it
			print FO "e(\"Edge $id\", [], 
                                    l(\"$o $id\", n(\"$o $id\", 
                                                   [a(\"OBJECT\", \"$text\")], 
                                                   [\n";

			$id++;
			$c->{'dvid'} = $id;
			# now, print our children, if any

			printChildren($c, $cg);

			print FO "]))),\n";
		}
	}
}

sub printNode {
	my $n = shift;
	my $cg = shift;

	my $o = "$n->{'f'} $n->{'thr'}";
	
	# first we print the root node:
	# [ l(name, n(name, [attrib], [...])) ]

	print FO " l(\"$o $id\", n(\"$o $id\", 
                                [a(\"OBJECT\", \"$o\")], [";

	$id++;

	# if there are children, we will print them where
	# the ... are.

	printChildren($n, $cg);

	print FO "]))";
}

