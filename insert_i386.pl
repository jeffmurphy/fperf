#!/usr/local/bin/perl -w
#
# insert.pl [.S file]
#
# Copyright (c) 1998, 1999 Nickel City Software

use strict;

my($debug) = 0;
$debug = 1 if defined($ENV{'FPERFDEBUG'});

print "fperf/insert.pl v1.0 (c) 1999 Nickel City Software\n";

my($file) = shift;
open(I, $file) || die "open($file): $!";
my ($tfile) = ${file}."_tfile";
open(O, "> $tfile") || die "open($tfile): $!";

my ($mainflag, $globlmain) = (0, 0);

# Options: S
#   S = perform startup routine
#   E = perform exit routine

my (%control);
if(defined($ENV{'FPERFOPTS'})) {
  foreach (split(/:/, $ENV{'FPERFOPTS'})) {
    $control{$_} = 1;
  }
} else {
  %control = ('S' => 1,
	      'E' => 1);
}

my ($lastFuncSeen) = "";

while(<I>) {
  chomp;

  # if we find ".globl main" it means that this file has
  # a global symbol called "main". good. this will most likely
  # be the executables entry point.

  if(/^\.globl main$/) {
    print "\t\"main\" is global.\n" if $debug;
    $globlmain = 1;
    print O "$_\n";
  }

  # have we found a function?

  elsif(/\.type\s+(\S+),\@function$/) {
    print "\tfound function \"$1\"\n" if $debug;
    $lastFuncSeen = $1;
    print O "$_\n";
  }

  # if we've found a label, analyze it. 

  elsif(/^(\S+):$/) {

    # if it is "main:" then we
    # insert some startup code (provided main is global) else we just
    # print out the label and keep going.

    if(($1 eq "main") && ($globlmain == 1)) {
      if($mainflag == 0) {
	$mainflag = 1;
	if(defined($control{'S'})) {
	  print "\tfound global \"main\" .. inserting fperfsetup() call.\n"
	    if $debug;
	  print O "$_\n";
	  print O "\n";
	  print O "\tpushl  %ebp\n";
	  print O "\tmovl   %esp, %ebp\n";
	  print O "\tsubl   \$0x14,%esp\n";
	  print O "\tpusha\n";
	  print O "\tmovl   0xc(%ebp),%eax\n";
	  print O "\tpushl  %eax\n"; # ac
	  print O "\tmovl   0x8(%ebp),%eax\n";
	  print O "\tpushl  %eax\n"; # av
	  print O "\tcall   fperfsetup\n";
	  print O "\taddl   \$8,%esp\n";
	  print O "\tpopa\n";
	  print O "\taddl   \$0x14,%esp\n";
	  print O "\tpopl   %ebp\n";
	} else {
	  print "\tfound global \"main\" but S option not specified.\n"
	    if $debug;
	}
      } else {
	die "\tWhoops. We've found two 'main:' labels?";
      }
    } 

    elsif($1 eq $lastFuncSeen) {
      # hmm. is this label the same as the last function declaration
      # we saw? if so, we should make a call to "fperffs" to note 
      # the start of a new function.

      print "\tfound start of function \"$1\", inserting fperffs() call\n"
	if $debug;
      print O "$_\n"; # label
      print O "\tpusha\n";
      print O "\tcall fperffs\n";
      print O "\tpopa\n";
    } 

    else {
      # if we aren't sure what this label is, we just print it
      # and keep going.
      print O "$_\n";
    }
  }

  # if the line is ".size main,..." then this is probably the
  # end of our "main" routine. we'll want to turn off our
  # "mainflag" so we don't incorrectly call "chkexit" when we
  # find "ret" instructions.

  elsif(/^\s*\.size\s+main,.*$/) {
    $mainflag = 0;
    print O "$_\n";
  }

  # if we've found a "ret" instruction and are confident that it
  # is part of our "main" routine then we can expect it to cause
  # the program to exit. therefor, call our cleanup routine first.

  elsif(($mainflag == 1) && (/^\s*ret\s*$/)) {
    print "\tfound 'ret' in main() .. " if $debug;
    if(defined($control{'E'})) {
      print "inserting fperfexit() call.\n" if $debug;
      print O "\tpusha\n";
      print O "\tcall fperfexit\n";
      print O "\tpopa\n";
    } else {
      print "but E option not specified.\n" if $debug;
    }
    print O "\t$_\n"; # ret
  }

  # if we've found a "ret" instruction and it isn't part of our "main"
  # routine then we need to update out stats by calling "fperffe"

  elsif(($mainflag == 0) && (/^\s*ret\s*$/)) {
    print "\tfound 'ret' in $lastFuncSeen - calling fperffe()\n"
      if $debug;
    print O "\tpusha\n";
    print O "\tcall fperffe\n";
    print O "\tpopa\n";
    print O "\t$_\n"; # ret
  }

  # if we are calling exit, lets cleanup first

  elsif(/^\s*call\s+exit\s*$/) {
    print "\tfound 'call exit' .. " if $debug;
    if(defined($control{'E'})) {
      print "inserting fperfexit() call.\n" if $debug;
      print O "\tcall fperfexit\n";
    } else {
      print "but E option not specified.\n" if $debug;
    }
    print O "$_\n"; # exit
  } 

  # finally: we don't know what this is, so just print it
  # out and move on.

  else {
    print O "$_\n";
  }
}

close(O);
close(I);

#rename $file, $file."_orig" || die "rename to _orig: $!";
unlink($file) || die "unlink($file): $!";
rename $tfile, $file || die "rename from temp: $!";

#system("cp $file ${file}-copy");

exit 0;

sub so($) {
  my ($t) = shift;
  return 1 if($t eq "b");
  return 2 if($t eq "w");
  return 4 if($t eq "l");

  die "unknown type ($t) can't figure out 'sizeof'";
}
