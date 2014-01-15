#!/usr/bin/perl
use warnings;

sub apply_vars;
sub die2;
sub usage
{
	die "Usage: $0 [--ucc=path] file\n";
}

sub timeout
{
	my $r = system("./timeout", '1', @_);
	return $r;
}

sub basename
{
	my $s = shift;
	return $1 if $s =~ m;/([^/]+$);;
	return $s;
}

my $ucc = '../ucc';
my $file = undef;
my $verbose = 0;
my $keep_temps = 0;

for(@ARGV){
	if(/^--ucc=(.+)/){
		$ucc = $1;
	}elsif($_ eq '-v'){
		$verbose = 1;
	}elsif($_ eq '--keep'){
		$keep_temps = 1;
	}elsif(!defined $file){
		$file = $_;
	}else{
		usage();
	}
}

(my $target = basename($file)) =~ s/\.[a-z]+$/.out/;
$target = "./$target";

END {
	unlink $target if defined $target and not $keep_temps;
}

my %vars = (
	's'         => $file,
	't'         => $target,
	'ucc'       => $ucc,
	'check'     => './check.sh',
	'asmcheck'  => './asmcheck.pl',
	'output_check' => './stdoutcheck.pl',
	'ocheck'    => './retcheck.pl',
	'layout_check' => './layout_check.sh',
	'caret_check' => './caret_check.pl',
	'debug_check' => './debug_check.pl',
);

if($verbose){
	my @verbose_support = (
		'check', 'ocheck', 'debug_check'
	);

	$vars{$_} .=  " -v" for @verbose_support;
}

# sanity checks:
my $ran = 0;
my $want_check = 0;
my $had_check = 0;

# export for sub-programs
$ENV{UCC} = $ucc;

open F, '<', $file or die2 "$file: $!";
while(<F>){
	chomp;

	if(my($command, $sh) = m{// *([A-Z]+): *(.*)}){
		if($command eq 'RUN'){
			$ran++;

			my $subst_sh = apply_vars($sh);
			print "$0: run: $subst_sh\n" if $verbose;

			my $want_err = ($subst_sh =~ s/^ *! *//);

			my $ec = timeout($subst_sh);

			die2 "command '$subst_sh' failed" if ($want_err == !$ec);
		}elsif($command eq 'CHECK'){
			$want_check = 1;

		}else{
			#die2 "unrecognised command: $command";
		}
	}
}
close F;

if($ran){
	if($want_check and not $had_check){
		die2 "CHECK commands found with no %check";
	}
	exit 0;
}else{
	die2 "no commands in $file";
}

# --------

sub die2
{
	die "$0: @_\n";
}

sub apply_vars
{
	my $f = shift;

	for my $regex ("^()", "([^%])"){
		$f =~ s/$regex%([a-z_]+)/
		$vars{$2} ? $1 . $vars{$2} : die2 "undefined variable %$2 in $file"/ge;

		$had_check++ if defined $2 and $2 eq 'check';
	}

	$f =~ s/%%/%/g;

	return $f;
}
