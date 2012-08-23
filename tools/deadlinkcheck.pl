#! /usr/bin/env perl

##### License information
# Copyright (C) 1999 Martial MICHEL
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the  Free Software Foundation; either  version  2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it  will be useful, but
# WITHOUT  ANY   WARRANTY;   without  even the   implied   warranty of
# MERCHANTABILITY or FITNESS  FOR A PARTICULAR  PURPOSE.   See the GNU
# General Public License for more details.
#
# More license information :
# http://www.gnu.org/copyleft/gpl.html

#####
# Dead Link Check  (DLC)  : HTTP link  checker  written in  Perl.  Can
# generate HTML output for easy checking of results and process a link
# cache file to hasten   multiple  requests. Initially created as   an
# extension to Public Bookmark Generator (PBM); can be used alone.

#####
# Script created by Martial MICHEL
# v0.0   : April 1st, 1999
# v0.1   : April 12th, 1999
# v0.1.1 : April 15th, 1999
# v0.1.2 : April 16th, 1999
# v0.2   : May 10th, 1999
# v0.2.1 : May 11th, 1999
# v0.3   : July 21st, 1999
# v0.3.1 : October 4th, 1999
# v0.3.2 : October 6th, 1999
# v0.4.0 : December 7th, 1999
# DLC questions/improvements can be directed to : martial@users.sourceforge.net
# DLC web-page : http://dlc.sourceforge.net/

#####
# Acknowledgments :
# Here are the list of person who helped improve this script, and that
# I wish to thank.
# v0.1 :  Marc Bednarek and   Jimmy Graham for helping  solving tricky
# little response  codes  and providing  me   with some examples   and
# information on how to resolve or do some http requests.
# v0.1.1    :  Wojciech Zwiefka    for  reporting  a  bug on  infinite
# redirection loop.
# v0.1.2 : Geoffrey Leach   for reporting a bug on  Timeout/maxTimeout
# interaction
# v0.3 : Josha Foust for reporting an http to ftp redirect bug.
# v0.3.1 : Josha Foust for reporting a redirect bug,  and a bug on url
# naming
# v0.3.2 : Olivier Galibert for reporting the lowercased URLs bug
# v0.4.0 :  The sourceforge team for the  fantastic job they are doing
# providing Open Source Coders such facilities.

#### Version info
$version = "0.4.0";

########################################
#####
# Documentation

#-*-Perl-*-

=head1 NAME

deadlinkcheck - Dead Link Check (v0.4.0)

=head1 SYNOPSIS

B<deadlinkcheck>
S<[B<-help>] [B<-verb> | B<-Verb> [B<-indicator>]]
[B<-proxy> I<proxy> | B<-Proxy>]
[[B<-timeCache> I<value>] | [B<-noCache>]]
[B<-Timeout> I<value>[I<:maxvalue>]] [B<-later> [I<percent>]]
[B<-userRedirect>] [B<-Content> [I<rule>[I<:...>]]]
[[B<-output> [I<filename>]] [B<-splitOutput>]]
[B<-rawOutput> | B<-detOutput>] [B<-codeConversion>] [B<-HTMLoutput>]
[B<-Dif>] I<urls> | I<filename>>

=head1 WARNING

I<deadlinkcheck> is a program that is still being evolved. Current version is already stable, but may not be fully functional.

=head1 BETA STATUS

The option I<-Content> (introduced in v0.4.0) is considered beta.

=head1 DESCRIPTION

I<Dead Link Check> (I<DLC>) is a I<Perl> script designed to find information on validity of HTTP references. The script may use/generate a cache file for avoiding redoing network requests if the user want to check added entries. The script works by reading entries from a file (or a list of links from the command line) and output results in file(s) (or I<STDOUT>).

I<DLC> was created as an extension to I<Public Bookmark Generator> (I<PBM>), but can be used by itself.

=head1 OPTIONS POSSIBLE VALUES

To obtain options values and options default values, run S<I<deadlinkcheck> I<-help>>

=head1 OPTIONS

=over 4

=item B<-help>

print a description of all the I<deadlinkcheck> options.

=item B<-verb>

run the script in verbose mode, printing advanced information on I<STDERR>.

=item B<-Verb>

run the script in maximum verbose mode, printing advanced run information on I<STDERR>.

=item B<-indicator>

show a progress indicator (in percent) at the beginning of the verbose output indicator.

=item B<-proxy> I<proxy>

set the I<http> and I<ftp> proxy to I<proxy>.

=item B<-Proxy>

get proxy information from environment variables (I<http_proxy>, I<ftp_proxy>).

=item B<-timeCache> I<value>

will set number of trusted days for links cache to I<value>.
Using a I<value> of S<0> will delete every entry in the cache.

=item B<-noCache>

will check links without reading/writing information from/to cache file.

=item B<-Timeout> I<value>[I<:maxvalue>]

set network timeout for requests to I<value>.
In case of retry, the script may extend the timeout value to I<maxvalue>.

=item B<-later> [I<percent>]

some requests to I<ftp> links may not be successful at a certain time. This option allow to retry the links later in the processing. The I<percent> optional argument is to force to retry the ftp request every I<percent> (or so).

=item B<-userRedirect>

will follow user HTTP-EQUIV redirections. Will force a GET to see if the user did not use a META command to force a redirection.

=item B<-Content> [I<rule>[I<:...>]]

will parse the content of the checked web page for information that may indicate that the page is an error return (some web servers do not always return proper HTTP responses) and/or was moved.
This option is based on the use of rules. Those rules can be of two types : I<error> or I<move>. You can use those type names to force the use of all I<error> or I<move> rules.
The I<help> rule is used to provide information on all the selected rules.

=Item B<-output> [I<filename>]

will output results into I<filename> if option is set.
Print to STDOUT if output option is not used.
If no I<filename> is provided, will save results to I<default option value>.

=item B<-splitOutput>

will output results into different files according to the first number of their HTTP return code.

=item B<-rawOutput>

will output only raw HTTP addresses.
It is recommended to use this output mode only if output is saved into split files.

=item B<-detOutput>

will output detailed results.

=item B<-codeConversion>

will print text instead of return code in a detailed output mode.

=item B<-HTMLoutput>

will output results as HTML code (easier to follow links).

=item B<-Dif>

will not print the "DLC information Footer" (added at the end of HTML outputs).

=back

=head1 RESTRICTIONS

=head2 SUPPORTED HTTP REQUESTS

As of now only links starting with S<file:/>, S<ftp://>, and S<http://> are supported.

=head1 SPECIAL CONCEPTS

=head2 INPUT LINKS

Links can be given to the script in two forms; on the command line, or from an input file.
The input file may be user provided or I<Public Bookmark Generator> created.
The input file may contain up to two information per line, the second being optional. Both information must be separated by a tabulation. Those informations are S<HTTPlink> and S<HTTPname>, where the first one is a fully qualified HTTP reference, and the second one is the name to be printed in reference to this link.

I<Public Bookmark Generator> created file fills the second field by the fully qualified name in the bookmark list (folders are separated by S<|>). 

Example :
S<|Work Informations|Developpements|Public Bookmark Generator>

=head2 READING MAXIMUM VERBOSE OUTPUT

When using the I<Maximum Verbose> option, a few informations are printed. In order :

S<[ I<xxx.xx> % ]> : a progress indicator (if I<indicator> option is selected).

S<*> : means that the script is retrying a S<ftp> site (I<later> option is selected).

S<(I<url>)> : indicates the I<url> being checked at that time.

S<@> : indicates that the script is doing a network connection (by using the I<cache>, some already checked network connections can be avoided).

S<[I<return code and action>]> : indicates the return code for this request, and optionally, actions to be taken.

Example :

 [  80.00 % ] * (ftp://ftp.redhat.com/) @ [401 -> Retry later]

means that the script already processed 80 % of the provided urls, is retrying a ftp site which url is S<ftp://ftp.redhat.com/> by doing a network connection, and the result of this request is S<401> (Unauthorized) so the action to be taken is to S<Retry later>.

=head2 LINKS CACHE

I<Dead Link Check> uses a link cache file to fasten access to the same link in case of multiple run.
The cache file (stored in the directory of the run) contains a time stamp information so that after a certain time, the links are not valid anymore.

=head2 RETURN CODES

S<RFC 2616> tells us that :

Informational S<1xx>
 I<100> is I<Continue>,
 I<101> is I<Switching Protocols>,

Successful S<2xx>
 I<200> is I<OK>,
 I<201> is I<Created>,
 I<202> is I<Accepted>,
 I<203> is I<Non-Authoritative Information>,
 I<204> is I<No Content>,
 I<205> is I<Reset Content>,
 I<206> is I<Partial Content>,

Redirection S<3xx>
 I<300> is I<Multiple Choices>,
 I<301> is I<Moved Permanently>,
 I<302> is I<Moved Temporalily>,
 I<304> is I<Not Modified>,
 I<305> is I<Use Proxy>,
 I<307> is I<Temporary Redirect>,

Client Error S<4xx>
 I<400> is I<Bad Request>,
 I<401> is I<Unauthorized>,
 I<403> is I<Forbidden>,
 I<404> is I<Not Found>,
 I<405> is I<Method Not Allowed>,
 I<406> is I<Not Acceptable>,
 I<407> is I<Proxy Authentication Required>,
 I<408> is I<Request Time-out>,
 I<409> is I<Conflict>,
 I<410> is I<Gone>,
 I<411> is I<Length Required>,
 I<412> is I<Precondition Failed>,
 I<413> is I<Request Entity Too Large>,
 I<414> is I<Request-URI Too Large>,
 I<415> is I<Unsupported Media Type>,
 I<416> is I<Requested range not satisfiable>,
 I<417> is I<Expectation Failed>,

Server Error <5xx>
 I<500> is I<Internal Server Error>,
 I<501> is I<Not Implemented>,
 I<502> is I<Bad Gateway>,
 I<503> is I<Service Unavailable>,
 I<504> is I<Gateway Time-out>,
 I<505> is I<HTTP Version not supported>.

A special return code (I<399>) has been introduced to handle I<user moved> web pages (using HTTP-EQUIV in the HTML source code) when the option B<-userRedirect> is set.

Another special return code (I<398>) is used when detecting an infinite loop redirection.

A redirect code (I<397>) may be seen when the option B<-Content> is used with at least one rule of type I<move>. It tells that the web page B<may> have moved.

A special page not found code (I<499>) may be seen when the option B<-Content> is used with at least one rule of type I<error>. It tells that the web page B<may> not exist.

Some I<ftp> sites may return I<400> and I<401> codes, still they may exist (they may just be unavailable at the time of the request).

Some I<http> sites may return I<500> code, still they may exist (the site may have not been available before timeout).

Note that if the script encounter return codes not defined in S<RFC 2616>, it will output those links in a special section.

=head2 PROXY

Use of a WWW cache server can be done using either the B<-proxy> or the B<-Proxy> option. The first one will read the proxy server from the command line, the second one will extract it from the environment variables (I<http_proxy> and I<ftp_proxy>).

If no proxy option is used, the script will run without proxy.

=head2 RULES

This option is considered in beta status.

The rules are based on regular expression parsing of the content of the web page checked. This slows down the processing of DLC, and may not return a proper result, it is recommanded to check the web page to verify.
Since it is based on text processing, it can only recognize entries for which it has rules, and may well miss some error or moved links.

Extending the rules is an easy process if you know how to use regular expressions in Perl, and are willing to edit the code of "deadlinkcheck". Inside the source code is a section called "How to create a rule ?" which should help you create (or modify) some rules. If you do so, please send a diff (or simply the function) by e-mail to S<martial@users.sourceforge.net>.

=head1 HOMEPAGE

You can find the I<Dead Link Check> homepage at : S<http://dlc.sourceforge.net/>. You may also want to check I<Public Bookmark Generator>, which web page is S<http://pbm.sourceforge.net/>.

=head1 BUGS

For bug reporting please send an e-mail to the author at I<martial@users.sourceforge.net> with S<[DLC]> in the title.

=head1 ACKNOWLEDGMENTS

Here are the list of person who helped improve this script, and that the author wish to thank :

=over 4

=item v0.1

I<Marc Bednarek> and I<Jimmy Graham> for helping solving tricky little response codes and providing me with some examples and information on how to resolve or do some http requests.

=item v0.1.1

I<Wojciech Zwiefka> for reporting a bug on infinite redirection loop.

=item v0.1.2

I<Geoffrey Leach> for reporting a bug on Timeout/maxTimeout interaction.

=item v0.3

I<Josha Foust> for reporting an http to ftp redirect bug.

=item v0.3.1

I<Josha Foust> for reporting a redirect bug, and a bug on url naming

=item v0.3.2

I<Olivier Galibert> for reporting the lowercased URLs bug

=item v0.4.0

The sourceforge team for the fantastic job they are doing providing Open Source Coders such facilities.

=back

=head1 LICENSE

Copyright (C) 1999 I<Martial MICHEL>

This program is free software; you can redistribute it and/or modify it under the terms of the I<GNU General Public License> as published by the I<Free Software Foundation>; either  version  2 of the License, or (at your option) any later version.

This program is distributed in the hope that it  will be useful, but I<WITHOUT  ANY   WARRANTY>;   without  even the   implied   warranty of MERCHANTABILITY or FITNESS  FOR A PARTICULAR  PURPOSE.   See the GNU General Public License for more details.

More license information : S<http://www.gnu.org/copyleft/gpl.html>

=head1 RELEASES

 v0.0   : April 1st, 1999
 v0.1   : April 12th, 1999
 v0.1.1 : April 15th, 1999
 v0.1.2 : April 16th, 1999
 v0.2   : May 10th, 1999
 v0.2.1 : May 11th, 1999
 v0.3   : July 21st, 1999
 v0.3.1 : October 4th, 1999
 v0.3.2 : October 6th, 1999
 v0.4.0 : December 7th, 1999

=head1 AUTHOR

Martial MICHEL (I<martial@users.sourceforge.net>)

=cut

########################################
#####
# Script

use Getopt::Long;
$Getopt::Long::autoabbrev = 1;
$Getopt::Long::ignorecase = 0;

require LWP::Protocol::http;
require LWP::UserAgent;

########################################
### Main Code

#### Default options
$verb = "F";
$input = "F";
$output = "F";
$outputfilename = "";
$defaultoutputfilename = "DLC_result";
$filename= "";
$defaultmaxtimeCachevalue = 10;
$maxtimeCachevalue = 0;
$Timeout = 5;
$maxTimeout = 180;
$noCache = "F";
$rawOutput = "F";
$detOutput = "F";
$codeConversion = "F";
$splitOutput = "F";
$dolater = "F";
$defaultdolatervalue = 10;
$dolatervalue = $defaultdolatervalue;
$userRedirect = "F";
$Proxy = "F";
$proxy = "";
$HTMLoutput = "F";
$Pindicator = "F";
$Dif = "T";

# Default variables
%urlchecked = ();
%redirect = ();
%infile = ();
@urllist = ();
$doget = "F";
$hostfile = ".dlchosts";
@supportedurls = ( "http", "ftp", "file" );

# content Rules DB
%RulesDB = ();
&RulesDBmaker();
@possibleMoveRules  = ();
@possibleErrorRules = ();
&RulesDBsplitter();
@allContentRules = (@possibleMoveRules, @possibleErrorRules);
@usedContentRules = ();

#### Arguments processing
SetMessages();
ProcessArguments(@ARGV);

#### Tool information
print STDERR $authlicense if ($verb ne "F");

#### Get Host file if existing
$timeCachevalue = time;
processhostfile();

#### Init variables
$tmpTimeout = $Timeout;
%urlname = ();
@filecontent = ();
$ua = new LWP::UserAgent;
$ua->agent("Mozilla/5.0 QunarBot/1.0 DeadLinkCheck_$version" . $ua->agent);
$ua->env_proxy() if ($Proxy eq "T");
$ua->proxy(['http', 'ftp'], $proxy) if ($proxy ne "");
## Existing + added status code
# Redirect (must start with a "3")
$contentMoveCode  = "397"; # "we have moved"
$infiniteLoopCode = "398";
$userRedirectCode = "399"; # HTTP-EQUIV
# Error (must start with a "4")
$contentPNFerrorCode  = "499"; # "Page Not Found"
%statuscode =
  # From RFC 2616 + redirect
  (
   # 1xx : Informational
   "100" => "Continue",
   "101" => "Switching Protocols",
   # 2xx : Successful
   "200" => "OK",
   "201" => "Created",
   "202" => "Accepted",
   "203" => "Non-Authoritative Information",
   "204" => "No Content",
   "205" => "Reset Content",
   "206" => "Partial Content",
   # 3xx : Redirection
   "300" => "Multiple Choices",
   "301" => "Moved Permanently",
   "302" => "Moved Temporalily",
   "304" => "Not Modified",
   "305" => "Use Proxy",
   "307" => "Temporary Redirect",
   $contentMoveCode  => "Possibly Moved by User",
   $infiniteLoopCode => "Infinite Loop", 
   $userRedirectCode => "User Redirect",
   # 4xx : Client Error
   "400" => "Bad Request",
   "401" => "Unauthorized",
   "403" => "Forbidden",
   "404" => "Not Found",
   "405" => "Method Not Allowed",
   "406" => "Not Acceptable",
   "407" => "Proxy Authentication Required",
   "408" => "Request Time-out",
   "409" => "Conflict",
   "410" => "Gone",
   "411" => "Length Required",
   "412" => "Precondition Failed",
   "413" => "Request Entity Too Large",
   "414" => "Request-URI Too Large",
   "415" => "Unsupported Media Type",
   "416" => "Requested range not satisfiable",
   "417" => "Expectation Failed",
   $contentPNFerrorCode => "Possible \"Page Not Found \"",
   # 5xx : Server Error
   "500" => "Internal Server Error",
   "501" => "Not Implemented",
   "502" => "Bad Gateway",
   "503" => "Service Unavailable",
   "504" => "Gateway Time-out",
   "505" => "HTTP Version not supported"
  );
@status = keys %statuscode;
@resok = ();
@resredirect = ();
@resnotfound = ();
@reserror = ();
@resmisc = ();
$lev0 = ($rawOutput eq "F") ? "| " : "";
$lev1 = ($rawOutput eq "F") ? "|-> " : "";
$lev2 = ($rawOutput eq "F") ? "| |-> " : "";
@todolist = ();
@splitlist = ("2xx", "3xx", "4xx", "5xx", "xxx");
@splitfn = ();
@todolater = ();
%BadHostName = ();
$BadHostNameCode = 500;
$pstring = "";

print STDERR "Processing files ..." if ($verb ne "F");
# Open files
if ($input eq "T") { # if 
  open INFILE, "<$filename"
    or die "Error opening $filename in Input mode : $!\n";
  @filecontent = <INFILE>;
  chomp @filecontent;
  close INFILE;
}
if ($output ne "F") { #if 
  if ($splitOutput eq "T") {
    foreach $val (@splitlist) {
      my $fn =  ($HTMLoutput eq "T") 
	? "$outputfilename\_$val.html" : "$outputfilename.$val";

      push @splitfn, $fn;
    }
  } else {
    $outputfilename = "$outputfilename.html"
      if (($HTMLoutput eq "T") && ($outputfilename eq $defaultoutputfilename));

    open OUTFILE, ">$outputfilename"
      or die "Error opening $outputfilename in output mode : $!\n";
  }
} else { #else
  *OUTFILE = STDOUT;
}
print STDERR " done\n" if ($verb ne "F");

if ($input eq "T") { # if 
  print STDERR "Extracting information " if ($verb ne "F");
  foreach $val (@filecontent) { # while
    my (@tmpsplit);
    
    @tmpsplit = split(/\t/, $val);
    
    push @urllist, $tmpsplit[0];
    $urlname{$tmpsplit[0]} = $tmpsplit[1] if ($tmpsplit[1] ne "");
    
    print STDERR "." if ($verb ne "F");
  }
  print STDERR " done\n" if ($verb ne "F");
}

### Checking URL List
print STDERR "Checking URL List " if ($verb ne "F");
@tmpurllist = @urllist;
@urllist = ();
foreach $url (@tmpurllist) {
  $url = urllc($url);

  if (($url =~ m%^(\w+?):%) && (! grep /^$1$/, @supportedurls)) {
    warn "Warning : Unsupported url address ($url)\n";
  } else {
    push @urllist, $url;
  }
    
  print STDERR "." if ($verb ne "F");
}
@tmpurllist = ();
print STDERR " done\n" if ($verb ne "F");

### Analyzing data
print STDERR "Analyzing data " if ($verb ne "F");
$inc = 0;
$rinc = 0; # real increment
$pinc = $dolatervalue;
$tmpcalc = 0;
$maxpercent = scalar @urllist;
foreach $url (@urllist) {
  my ($tmpurl);
  my $ntdl = scalar @todolater;
  local %tmplookurl = ();

  ### Every $dolatervalue (or more)
  if ($tmpcalc >= $pinc) {
    $pinc += $dolatervalue;
    
    ### Do later
    if ($ntdl > 0) {
      @tmpdolater = @todolater;
      @todolater = ();
      
      while ($tmpurl = shift @tmpdolater) {
	if ($Pindicator eq "T") {
	  $rinc = $inc - (scalar @tmpdolater + scalar @todolater);
	  $tmpcalc = ($rinc / $maxpercent) * 100;
	  
	  $pstring = sprintf "[ %6.2f \% ] * ", $tmpcalc;
	}
	
	&netdoit($tmpurl);
      }
    }
  }

  print STDERR "." if ($verb eq "T");

  $rinc = ++$inc - scalar @todolater;
  $tmpcalc = ($rinc / $maxpercent) * 100;

  $pstring = sprintf "[ %6.2f \% ] ", $tmpcalc
    if ($Pindicator eq "T");

  &netdoit($url);
}

### Do later
$dolater = "F";
if (scalar @todolater > 0) {
  foreach $url (@todolater) {
    $tmpcalc = (++$rinc / $maxpercent) * 100;
    $pstring = sprintf "[ %6.2f \% ] * ", $tmpcalc
      if ($Pindicator eq "T");

    &netdoit($url);
  }
}
print STDERR "\n" if ($verb eq "X");
print STDERR " done\n" if ($verb ne "F");

### Creating results
print STDERR "Creating results " if ($verb ne "F");
$redon = "F";

# ok
foreach $url (@urllist) {
  if (($urlchecked{$url} =~ /^2/) && (grep /^$urlchecked{$url}/, @status)) {
    &printurl(\@resok);
  } else {
    push @todolist, $url;
  }
}
print STDERR "." if ($verb ne "F");

# Redirect
@urllist = @todolist;
@todolist = ();
$redon = "T";
foreach $url (@urllist) {
  if (exists $redirect{$url}) {
    &printurl(\@resredirect);
  } else {
    push @todolist, $url;
  }
}
$redon = "F";
print STDERR "." if ($verb ne "F");

# Not found
@urllist = @todolist;
@todolist = ();
foreach $url (@urllist) {
  if (($urlchecked{$url} =~ /^4/) && (grep /^$urlchecked{$url}$/, @status)){
    &printurl(\@resnotfound);
  } else {
    push @todolist, $url;
  }
}
print STDERR "." if ($verb ne "F");

# Error
@urllist = @todolist;
@todolist = ();
foreach $url (@urllist) {
  if (($urlchecked{$url} =~ /^5/) && (grep /^$urlchecked{$url}$/, @status)) {
    &printurl(\@reserror);
  } else {
    push @todolist, $url;
  }
}
print STDERR "." if ($verb ne "F");

# Misc
foreach $url (@todolist) {
  &printurl(\@resmisc);
}
print STDERR "." if ($verb ne "F");

# All done
print STDERR " done\n" if ($verb ne "F");

##### Printing results
print STDERR "Printing results " if ($verb ne "F");
$splitfninc = 0;

# ok
&printresult("2xx", @resok);
$splitfninc++;
print STDERR "." if ($verb ne "F");

# Redirect
&printresult("3xx", @resredirect);
$splitfninc++;
print STDERR "." if ($verb ne "F");

# Not found
&printresult("4xx", @resnotfound);
$splitfninc++;
print STDERR "." if ($verb ne "F");

# Error
&printresult("5xx", @reserror);
$splitfninc++;
print STDERR "." if ($verb ne "F");

# Misc
&printresult("xxx", @resmisc);
print STDERR "." if ($verb ne "F");

# All done
print STDERR " done\n" if ($verb ne "F");

##### Script done
print STDERR "\n$0 : Finish\n" if ($verb ne "F");

########################################
### Sub routines

### Set Messages
sub SetMessages { # sub
  my $sp = "  ";

  $usage= << "END";
Usage : $0 [-help] [-verb | -Verb [-indicator] ] [-proxy proxy | -Proxy] [[-timeCache value] | [-noCache]] [-Timeout value[:maxvalue]] [-later [percent]] [-userRedirect] [-Content [rule[:...]]] [[-output [filename]] [-splitOutput]] [-rawOutput | [-detOutput [-codeConversion]]] [-HTMLoutput] [-Dif] urls | filename
${sp}-help : this help page
${sp}-verb : verbose mode
${sp}-Verb : Maximum verbose mode
${sp}-indicator : Progress indicator (in percent)
${sp}-proxy proxy : set http and ftp proxy to "proxy"
${sp}-Proxy : get proxy informations from environment variables
${sp}-timeCache value: if a host file exist, max time value (in days) to work with found address (default is $defaultmaxtimeCachevalue)
${sp}-noCache : will not read or store information in the cache file ($hostfile)
${sp}-Timeout value[:maxvalue] : network timeout value (default values : $Timeout and $maxTimeout)
${sp}-later [percent]: will keep a list of failed ftps to do later. Will retry after each "percent" (default value : $defaultdolatervalue %)
${sp}-userRedirect : will follow user HTTP-EQUIV redirections (force a GET better than a HEAD)
${sp}-Content [rule[:...]] : check content of web page for "move"d like text entries and/or "error" like text entries (possible "move" rules : @possibleMoveRules) (possible "error" rules : @possibleErrorRules) ("help" to get information on rules) ("all" for all "move" and all "error") (default : error) (Warning : BETA) 
${sp}-output [filename] : output result to filename (STDOUT otherwise) (if no filename provided, output to : $defaultoutputfilename)
${sp}-splitOutput : will output results in a files with base return code extension
${sp}-rawOutput : will print output without any other information (location in bookmark if provided)
${sp}-detOutput : will output detailed results
${sp}-codeConversion : will print text instead of return code
${sp}-HTMLoutput : will output results as HTML
${sp}-Dif : not to print the "DLC information Footer"
${sp}urls : URLs to analyze
${sp}filename : file to analyze

Supported URLs start with : @supportedurls

END
;

  $authlicense = << "END";
Dead Link Check (v$version)
Copyright (C) 1999 Martial MICHEL
Licensed under the GNU General Public License

END
;

$toolused= << "END";
Generated using <A HREF="http://dlc.sourceforge.net/">Dead Link Check</A> (v$version) by <A HREF="http://www.loria.fr/~michel/">Martial MICHEL</A>
END
;

} # SetMessages

#### Process the arguments
sub ProcessArguments { # sub
  my (@options, $val);
  local ($opt_help, $opt_verb, $opt_output, $opt_timeCache, $opt_Timeout,
	 $opt_noCache, $opt_rawOutput, $opt_detOutput, $opt_codeConversion,
	 $opt_splitOutput, $opt_later, $opt_userRedirect, $opt_Content,
	 $opt_proxy, $opt_Proxy, $opt_HTMLoutput, $opt_Dif, $opt_indicator);

  @options = (
	      "help",           # help
	      "verb",           # verbose
	      "Verb",           # Verbose
	      "proxy=s",        # proxy + 1 required string argument
	      "Proxy",          # proxy from env.
	      "timeCache=i",    # timeCache + 1 required integer argument
	      "noCache",        # noCache
	      "Timeout=s",      # Timeout + 1 req string (may contain 2 int)
	      "later:i",        # ftp dolater + 1 optional integer argument
	      "userRedirect",   # user HTTP-EQUIV redirections
	      "Content:s",      # content + 1 opt arg
	      "output:s",       # output + 1 optional string argument
	      "splitOutput",    # splitOutput
	      "rawOutput",      # rawOutput
	      "detOutput",      # detailed output
	      "codeConversion", # return code as text
	      "HTMLoutput",     # HTML file output
	      "Dif",            # DLC information footer
	      "indicator"       # Progress indicator
	     );
  die "\n$usage\n" unless GetOptions @options;

  die "$authlicense\n$usage\nMore information using \"perldoc deadlinkcheck\" or \"man deadlinkcheck\"\n"
    if (($opt_help) ||
	((scalar @ARGV == 0) && (! defined $opt_Content))
	);

  # Verbose & Progress indicator
  die "Error : Only one verbose mode allowed\n"
    if ((defined $opt_verb) && (defined $opt_Verb));

  $verb = "T" if (defined $opt_verb);
  $verb = "X" if (defined $opt_Verb);

  if (defined $opt_indicator) {
    die "Error : Progress indicator authorized only with maximum verbose\n"
      if ($verb ne "X");
    $Pindicator = "T";
  }

  # Proxy
  die "Error : Only one proxy option at a time\n"
    if ((defined $opt_proxy) && (defined $opt_Proxy));

  $proxy = $opt_proxy if (defined $opt_proxy);
  $Proxy = "T" if (defined $opt_Proxy);

  # File Output 
  if (defined $opt_output) {
    $output = "T";
    $outputfilename = ($opt_output eq "") ? $defaultoutputfilename : $opt_output;
    $outputfilename =~
    s{^~([^/]*)}{$1?(getpwnam($1))[7]:($ENV{HOME} || $ENV{LOGDIR})}ex;
  }

  # Cache related data
  if (defined $opt_timeCache) {
    $maxtimeCachevalue = $opt_timeCache * 24 * 3600;
  } else {
    $maxtimeCachevalue = $defaultmaxtimeCachevalue * 24 * 3600;
  }

  if (defined $opt_Timeout) {
    if ($opt_Timeout =~ /^(\d+):(\d+)$/) {
      $Timeout = $1;
      $maxTimeout = $2;
    } elsif ($opt_Timeout =~ /^(\d+)$/) {
      $Timeout = $1;
    } else {
      die "Error : Wrong timeout value ($opt_Timeout)\n";
    }
    die "Error : Timeout value ($1) must be inferior to max value ($maxTimeout)\n" if ($maxTimeout <= $Timeout);
  }

  $noCache = "T" if (defined $opt_noCache);

  # output look
  die "Error : only one output mode (raw or detailled) at a time\n"
    if ((defined $opt_rawOutput) && (defined $opt_detOutput));

  die "Error : codeConversion only available if detailled mode is selected\n"
    if ((! defined $opt_detOutput) && (defined $opt_codeConversion));

  die "Error : split option only available if output option selected\n"
    if ((defined $opt_splitOutput) && (! defined $opt_output));

  $rawOutput = "T" if (defined $opt_rawOutput);

  $detOutput = "T" if (defined $opt_detOutput);

  $codeConversion = "T" if (defined $opt_codeConversion);

  $HTMLoutput = "T" if (defined $opt_HTMLoutput);

  $splitOutput = "T" if (defined $opt_splitOutput);

  # dolater
  if (defined $opt_later) {
    die "Error : Possible values for \"do later\" are from 1 to 100 \%\n"
      if (($opt_later < 0) || ($opt_later > 100));
    $dolater = "T";
    $dolatervalue = ($opt_later == 0) ? $defaultdolatervalue : $opt_later;
  }

  # redirection
  $userRedirect = "T" if (defined $opt_userRedirect);

  # Content
  if (defined $opt_Content) {
    my ($help, $some) = ("F", "F");
    my ($error, $move) = ("F", "F");
    my ($all, $warn) = ("F", "F");
    my %tmpcontent, $val;

    foreach $val ( split(/:/, $opt_Content) ) {
      $help  = "T", next if ($val =~ m/^help$/);
      $error = "T", next if ($val =~ m/^error$/);
      $move  = "T", next if ($val =~ m/^move$/);
      $all   = "T", next if ($val =~ m/^all$/);
      $some  = "T";
      
      warn "Error : Inexistent rule \"$val\".\n$usage\n", $warn = "T"
	if (! grep /^$val$/, @allContentRules);
      
      $tmpcontent{$val} = "";
    }
    # die if any warn
    die "\n" if ($warn eq "T");

    # No rules provided = only error
    $error = "T" if ($some eq "F");

    # ALL
    ($error, $move) = ("T", "T") if ($all eq "T");

    # Add all "move" or "error" as required
    if ($error eq "T") {
      foreach $val (@possibleErrorRules) {
	$tmpcontent{$val} = "";
      }
    }
    if ($move eq "T") {
      foreach $val (@possibleMoveRules) {
	$tmpcontent{$val} = "";
      }
    }

    # Create @usedContentRules
    foreach $val (keys %tmpcontent) {
      push @usedContentRules, $val;
    }

    # Print help on requested rules
    &RulesHelp(@usedContentRules), die "\n"
      if ($help eq "T");

    $content = "T";
  }

  # Dif
  $Dif = "F" if (defined $opt_Dif);

  # remaining arguments
  if (scalar @ARGV == 1) {
    $val = shift @ARGV;
    $tmpval = $val;
    $tmpval =~ 
      s{^~([^/]*)}{$1?(getpwnam($1))[7]:($ENV{HOME} || $ENV{LOGDIR})}ex;
    if (-f $tmpval) {
      $input = "T";
      $filename = $tmpval;
    } else {
      push @urllist, $val;
    }
  } else {
    @urllist = @ARGV;
  }
}

sub processhostfile {
  my (@tmp, $val, $tval1, $tval2, $tval3);
  
  return if ($noCache eq "T");

  if (-e $hostfile) {
    print STDERR "Reading Host file " if ($verb ne "F");
    open TMP, "<$hostfile"
      or die "Error opening $hostfile in Input mode : $!\n";
    @tmp = <TMP>;
    chomp @tmp;
    close TMP;
  
    open TMP, ">$hostfile.tmp"
      or die "Error opening $hostfile.tmp in Output mode : $!\n";
    foreach $val (@tmp) {
      @tmpsplit = split(/\t/, $val);
      
      if (($timeCachevalue - $tmpsplit[0]) <= $maxtimeCachevalue) {
	$infile{$tmpsplit[1]} = "";
	$urlchecked{$tmpsplit[1]} = $tmpsplit[2];

	print TMP "$tmpsplit[0]\t$tmpsplit[1]\t$tmpsplit[2]";

	if ($tmpsplit[2] =~ /^3/) {
	  $redirect{$tmpsplit[1]} = $tmpsplit[3];
	  print TMP "\t$tmpsplit[3]";
	}

	print TMP "\n";

	print STDERR "." if ($verb ne "F");
      }
    }
    close TMP;
  
    unlink "$hostfile";
    rename "$hostfile.tmp", "$hostfile";
    print STDERR " done\n" if ($verb ne "F");
  }
}

#### Adding to host file
sub addhostfile {
  my $url = shift @_;
  my $rcode = shift @_;
  my $addinfo = shift @_; # Not always something

  return if ($noCache eq "T");
  return if (exists $infile{$url});

  open TMP, ">>$hostfile"
    or die "Error opening $hostfile in Add mode : $!\n";

  print TMP "$timeCachevalue\t$url\t$rcode";

  print TMP "\t$addinfo" if ($rcode =~ /^3/);

  print TMP "\n";
  
  close TMP;
}

##### netcheck
sub netcheck {
  my $url = shift @_;
  my ($tmpurl, $val);

  print STDERR "\n$pstring($url)" if ($verb eq "X");

  if ((exists $tmplookurl{$url}) && ($tmplookurl{$url} > 0)) {
    $urlchecked{$url} = $infiniteLoopCode;
    print STDERR "[", $urlchecked{$url}, " -> Infinite loop]"
      if ($verb eq "X");
    return;
  }

  if (exists $urlchecked{$url}) {
    if ($urlchecked{$url} !~ /^3/) {
      print STDERR "[", $urlchecked{$url}, "]" if ($verb eq "X");
      return;
    } else {
      $tmplookurl{$url}++;
      if (exists $redirect{$url}) {
	print STDERR "[", $urlchecked{$url}, " -> ", $redirect{$url}, "]"
	  if ($verb eq "X");
	&netcheck($redirect{$url});
	return;
      }
    }
  }

  ### From here it means we are doing a network request
  print STDERR " @ " if ($verb eq "X");
  $ua->timeout($tmpTimeout);

  if ($doget ne "T") {
    $request = new HTTP::Request(($url =~ /^http/) ? 'HEAD' : 'GET', $url);
  } else {
    $request = new HTTP::Request('GET', $url);
  }
  
  $response = $ua->simple_request($request, undef, undef);

  if ($response->is_redirect) { ##### Response is a redirection
    ### 3xx
    $tmpurl = $1 if ($response->as_string =~ m%Location:\s+(.*)%g);
    $tmpurl = makecorrecturl($url, $tmpurl);
    print STDERR "[", $response->code," -> ", $tmpurl, "]" if ($verb eq "X");
    $redirect{$url} = $tmpurl;
    $urlchecked{$url} = $response->code;
    &addhostfile($url, $response->code, $tmpurl);
    &netcheck($tmpurl);
    return;
  } elsif ($response->is_error) { ##### Response is an error
    ### 400 and 401
    if ($response->code =~ /^40[0|1]/) {
      if (($url =~ /^ftp/) || ($url =~ m%http://ftp%)) {
	if ($tmpTimeout == $Timeout) {
	  print STDERR "[", $response->code, " -> Retry]"
	    if ($verb eq "X");
	  $tmpTimeout = $maxTimeout;
	  &netcheck($url);
	  return;
	}
	if ($dolater eq "T") {
	  print STDERR "[", $response->code, " -> Retry later]"
	    if ($verb eq "X");
	  push @todolater, $url;
	  return;
	}
      }
    ### 403 and 404
    } elsif ($response->code =~ /^40[3|4]/) {
      if ($doget ne "T") {
	print STDERR "[", $response->code, " -> Retry (GET)]"
	  if ($verb eq "X");
	$doget = "T";
	&netcheck($url);
	return;
      }
    ### 500
    } elsif ($response->code == 500) {
      if ($response->as_string =~ m%Bad\shostname%) {
	print STDERR "[", $response->code, " (Bad hostname)]"
	  if ($verb eq "X");
	$BadHostName{&urlbase($url)} = "";
	$urlchecked{$url} = $response->code;
	&addhostfile($url, $response->code);
	return;
      }
      if ($tmpTimeout == $Timeout) {
	print STDERR "[", $response->code, " -> Retry]" if ($verb eq "X");
	if ($response->as_string =~ m%HTTP/1.1%) {
	  if ($doget ne "T") {
	    $doget = "T";
	    print STDERR "(GET)" if ($verb eq "X");
	  }
	}
	$tmpTimeout = $maxTimeout;
	&netcheck($url);
	return;
      } elsif ($doget eq "F") {
	print STDERR "[", $response->code, " -> Retry (GET)]"
	  if ($verb eq "X");
	$doget = "T";
	&netcheck($url);
	return;
      }
    ### 501
    } elsif ($response->code == 501) {
      if ($doget ne "T") {
	print STDERR "[", $response->code, " -> Retry (GET)]"
	  if ($verb eq "X");
	$doget = "T";
	&netcheck($url);
	return;
      }
    }
  ### Success
  } elsif ($response->is_success) {
    if ($userRedirect eq "T") {# User Redirect
      my $tmpurl = &getrefreshurl($response->headers_as_string);
      
      if ($tmpurl ne "") {
	$tmpurl = makecorrecturl($url, $tmpurl);
	$urlchecked{$url} = $userRedirectCode;
	print STDERR "[", $urlchecked{$url}, " -> $tmpurl]"
	  if ($verb eq "X");
	$redirect{$url} = $tmpurl;
	&addhostfile($url, $userRedirectCode, $tmpurl);
	&netcheck($tmpurl);
	return;
      }
    }
    if ($content eq "T") { # Content check
      my ($tmpcode, $tmpurl) =
	&checkcontent($url, $response->headers_as_string, $response->content);

      # If it is a move, check on the URL
      if ($tmpurl ne "") {
	$tmpurl = makecorrecturl($url, $tmpurl);
	$urlchecked{$url} = $contentMoveCode;
	print STDERR "[", $urlchecked{$url}, " -> $tmpurl]"
	  if ($verb eq "X");
	$redirect{$url} = $tmpurl;
	&addhostfile($url, $contentMoveCode, $tmpurl);
	&netcheck($tmpurl);
	return;
      }

      # If it is an error, check on the return code
      if ($tmpcode ne "") {
	print STDERR "[", $tmpcode, "]" if ($verb eq "X");
	$urlchecked{$url} = $tmpcode;
	&addhostfile($url, $tmpcode);
	return;
      }
    }
    ### End
  }

  ### This is when we got an answer (except for 3** and 500(bad hostname))
  print STDERR "[", $response->code, "]" if ($verb eq "X");

  $urlchecked{$url} = $response->code;
  &addhostfile($url, $response->code);
}

sub netdefaultset {
  $tmpTimeout = $Timeout;
  $doget = (($userRedirect eq "T") || ($content eq "T")) ? "T" : "F";
}

sub netdoit {
  my $url = shift @_;

  if (! exists $BadHostName{&urlbase($url)}) {
    &netdefaultset();
    &netcheck($url);
  } else {
    # We know that this host is a Bad Host Name ... no need to continue
    print STDERR "\n$pstring($url) [$BadHostNameCode (Bad hostname)]" if ($verb eq "X");
    $urlchecked{$url} = $BadHostNameCode;
    &addhostfile($url, $BadHostNameCode);
  }
}

#### Formatting options
sub printurl {
  my $array = shift @_;

  if ((exists $urlname{$url}) && ($rawOutput eq "F")) {
    push @{$array}, "<CODE>" if ($HTMLoutput eq "T");
    push @{$array}, "${lev1}";
    push @{$array}, "</CODE><A HREF=\"$url\">" if ($HTMLoutput eq "T");
    push @{$array}, "<B>"
      if (($HTMLoutput eq "T") && ($urlchecked{$url} == 200));
    push @{$array}, "$urlname{$url}";
    push @{$array}, "</B>"
      if (($HTMLoutput eq "T") && ($urlchecked{$url} == 200));
    push @{$array}, "</A>" if ($HTMLoutput eq "T");
    if ($detOutput eq "T") {
      push @{$array}, "<BR>" if ($HTMLoutput eq "T");
      push @{$array}, "\n";
      push @{$array}, "<CODE>" if ($HTMLoutput eq "T");
      push @{$array}, "${lev2}";
      push @{$array}, "</CODE>" if ($HTMLoutput eq "T");
      push @{$array}, "[$url]";
    }
  } else {
    if ($rawOutput eq "F") {
      push @{$array}, "<CODE>" if ($HTMLoutput eq "T");
      push @{$array}, "${lev1}";
      push @{$array}, "</CODE>" if ($HTMLoutput eq "T");
    }
    push @{$array}, "<A HREF=\"$url\">" if ($HTMLoutput eq "T");
    push @{$array}, "<B>"
      if (($HTMLoutput eq "T") && ($urlchecked{$url} == 200));
    push @{$array}, "$url";
    push @{$array}, "</B>"
      if (($HTMLoutput eq "T") && ($urlchecked{$url} == 200));
    push @{$array}, "</A>" if ($HTMLoutput eq "T");
  }

  if ($detOutput eq "T") {
    if (($codeConversion eq "T") 
	&& (exists $statuscode{$urlchecked{$url}})) {
      push @{$array}, " ($statuscode{$urlchecked{$url}})";
    } else {
      push @{$array}, " ($urlchecked{$url})";
    }
    push @{$array}, "<BR>" if ($HTMLoutput eq "T");
    push @{$array}, "\n";
  }

  # Redirect
  if ($redon eq "T") {
    my $tmpurl = $url;
    my $redok = "F";
    my %tmplookurl = ();

  RSW: {
      while ((exists $redirect{$tmpurl}) && ($tmpurl ne $redirect{$tmpurl})) {
	last RSW if ($tmplookurl{$tmpurl} > 0);
	$tmplookurl{$tmpurl}++;
	if (($detOutput eq "T") && ($redok eq "T")) {
	  push @{$array}, "<CODE>" if ($HTMLoutput eq "T");
	  push @{$array}, "${lev2}";
	  push @{$array}, "</CODE>" if ($HTMLoutput eq "T");
	  push @{$array}, "<A HREF=\"$tmpurl\">" if ($HTMLoutput eq "T");
	  push @{$array}, "<B>"
	    if (($HTMLoutput eq "T") && ($urlchecked{$tmpurl} == 200));
	  push @{$array}, "$tmpurl";
	  push @{$array}, "</B>"
	    if (($HTMLoutput eq "T") && ($urlchecked{$tmpurl} == 200));
	  push @{$array}, "</A>" if ($HTMLoutput eq "T");
	  if (($codeConversion eq "T") 
	      && (exists $statuscode{$urlchecked{$tmpurl}})) {
	    push @{$array}, " ($statuscode{$urlchecked{$tmpurl}})" ;
	  } else {
	    push @{$array}, " ($urlchecked{$tmpurl})";
	  }
	  push @{$array}, "<BR>" if ($HTMLoutput eq "T");
	  push @{$array}, "\n";
	}
	$tmpurl = $redirect{$tmpurl};
	$redok = "T";
      }
    }
    if ($detOutput eq "T") {
      push @{$array}, "<CODE>" if ($HTMLoutput eq "T");
      push @{$array}, "${lev2}";
      push @{$array}, "</CODE>" if ($HTMLoutput eq "T");
    } else {
      if ($HTMLoutput eq "F") {
	push @{$array}, "\t";
      } else {
	push @{$array}, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
      }
    }
    push @{$array}, "<A HREF=\"$tmpurl\">" if ($HTMLoutput eq "T");
    push @{$array}, "<B>"
      if (($HTMLoutput eq "T") && ($urlchecked{$tmpurl} == 200));
    push @{$array}, "$tmpurl";
    push @{$array}, "</B>"
      if (($HTMLoutput eq "T") && ($urlchecked{$tmpurl} == 200));
    push @{$array}, "</A>" if ($HTMLoutput eq "T");
    if ($detOutput eq "T") {
      if (($codeConversion eq "T") 
	  && (exists $statuscode{$urlchecked{$tmpurl}})) {
	push @{$array}, " ($statuscode{$urlchecked{$tmpurl}})" ;
      } else {
	push @{$array}, " ($urlchecked{$tmpurl})";
      }
    }
    push @{$array}, "<BR>" if (($HTMLoutput eq "T") && ($detOutput eq "T"));
    push @{$array}, "\n" if ($detOutput eq "T");
  } # end redirect

  # End
  if ($detOutput eq "T") {
    push @{$array}, "<CODE>" if ($HTMLoutput eq "T");
    push @{$array}, "${lev0}";
    push @{$array}, "</CODE>" if ($HTMLoutput eq "T");
  }
  push @{$array}, "<BR>" if ($HTMLoutput eq "T");
  push @{$array}, "\n";
} # printurl

sub printHTMLbeg {
  my $title = shift @_;

  print OUTFILE "<HTML>\n<TITLE>$title</TITLE>\n<BODY BGCOLOR=\"\#FFFFFF\">\n";
  print OUTFILE "<H1>$title</H1><BR><HR><BR>\n";
}

sub printHTMLend {
  print OUTFILE "<BR><HR>$toolused" if ($Dif eq "T");
  print OUTFILE "</BODY>\n</HTML>\n";
}

sub printresult {
  my $txt = shift @_;
  my @proc = @_;

  if (scalar @proc > 0) {
    if ($splitOutput eq "T") {
      open OUTFILE, ">$splitfn[$splitfninc]"
	or die "Error : Cannot open $splitfn[$splitfninc] for output : $!\n";
      &printHTMLbeg("DLC Results : $txt") if ($HTMLoutput eq "T");
    } else {
      &printHTMLbeg("DLC Results")
	if (($HTMLoutput eq "T") && ($splitfninc == 0));
    }

    
    if (($HTMLoutput eq "T") && ($splitOutput eq "F")) {
      print OUTFILE "<HR>\n" if ($splitfninc > 0);
    }
    if (($splitOutput eq "F") && ($rawOutput eq "F")) {
      print OUTFILE "\n\n";
      print OUTFILE "<H1>" if ($HTMLoutput eq "T");
      print OUTFILE "Entries with code $txt :";
      if ($HTMLoutput eq "T") {
	print OUTFILE "</H1>\n";
      } else {
	print OUTFILE "\n";
      }
    }
    foreach $val (@proc) {
      print OUTFILE $val;
    }
    if (($splitOutput eq "T") || ($splitfninc == 4)) {
      &printHTMLend() if ($HTMLoutput eq "T");
      close OUTFILE;
    }
  } else {
    if ($splitOutput eq "T") {
      unlink "$splitfn[$splitfninc]"
	if (-f "$splitfn[$splitfninc]");
    } else {
      if ($splitfninc == 4) {
	&printHTMLend() if ($HTMLoutput eq "T");
	close OUTFILE;
      }
    }
  }
} # printresult

# In case of user HTTP-EQUIV refresh to a new url
sub getrefreshurl {
  my $return = "";
  my $val = shift @_;

  $return = $1 if ($val =~ m%^refresh:.*?url=\'?(.+)\'?$%im);

  return ($return);
} # getrefreshurl

# In case of redirection, make a correct url if necessary
sub makecorrecturl {
  my $url    = shift @_;
  my $tmpurl = urllc (shift @_);
  my ($begurl, $endurl);

  if (($tmpurl !~ /^(ht|f)tp/) && ($url =~ /^http/)) {
    if ($tmpurl =~ m%^/(.*)$%) {
      $endurl = "$1";
      $begurl = "$1" if ($url =~ m%^(http://.*?)/%);
    } else {
      $begurl = "$1" if ($url =~ m%^(http://.*)/%);
      $endurl = "$tmpurl";
    }
    $tmpurl = "$begurl/$endurl";
  }
  
  return $tmpurl;
} # makecorrecturl

# Extract the url base
sub urlbase {
  my $tmp = shift @_;
  my $res = "";

  $res = $1 if ($tmp =~ m%^(\w+\:/.+?/)%);

  return ($res);
}

# Make a correct lowercase front part to the URL
sub urllc {
  my $tmp = shift @_;
  my $res = "";

  # Every URL machine should finish with a /
  $tmp = $tmp . "/"
    if ($tmp =~ m%^\w+\://?[^/]+?$%);

  # Every machine name is case insensitive (let's lowercase them)
  $res = (lc $1) . $2
    if ($tmp =~ m%^(\w+\:/.+?/)(.*)$%);

  # To have a minimal output
  $res = $tmp
    if ($res eq "");

  return ($res);
}

######################################################################
######################################################################
######################################################################

####################
# Content rules

# Check content of HTML files (header available)
sub checkcontent {
  local $url     = shift @_;
  local $header  = shift @_;
  local $content = shift @_;

  local $op = "check";
  local $returncode = "";
  local $returnurl  = "";

  my $val;

  foreach $val (@usedContentRules) {
    &{$RulesDB{$val}};

    return ($returncode, $returnurl)
      if (($returncode ne "") || ($returnurl ne ""));
  }

  return ("", "");
} # getmovedurl

### Printing Help entries for some rules
sub RulesHelp {
  local $op;
  my $val;
  my $type, $expl;

  print "\nHelp on rules :\n--------------------\n";

  foreach $val (@usedContentRules) {
    $op = "type";
    $type = &{$RulesDB{$val}};
    $op = "help";
    $expl = &{$RulesDB{$val}};

    print " - $val (type : $type) : $expl\n";
  }
}

### Splitting rules into "move" and "error"
sub RulesDBsplitter {
  local $op = "type";
  my $type;

  foreach $val (keys %RulesDB) {
    $type = &{$RulesDB{$val}};
    
    if ($type eq "move") {
      push @possibleMoveRules, $val;
    } elsif ($type eq "error") {
      push @possibleErrorRules, $val;
    } else {
      die "Internal Error : Rule \"$val\" is of no recognized type ($type)\n";
    }
  }
}

### Making the rules database
sub RulesDBmaker {
  # Error rules
  $RulesDB{"pnf"} = \&er_pnf;

  # Move rules
  $RulesDB{"hm"}  = \&mv_hm;
  $RulesDB{"hbm"} = \&mv_hbm;
}

### All rules are after here

## How to create a rule ?
# (You may want to check the default provided rules)
# 1) Each rules is to be placed inside a sub function which name should
#    start with "er_" or "mv_" ("error" or "move").
#    There is no argument to the functions, commands and variables are
#    used from "local" entries from calling procedure
#     $op          : operation to perform; 3 possibles values
#        "type"  : return type string ("error" or "move")
#        "help"  : return help string
#        "check" : perform check
#     $returncode  : code return field (change only if found) (error case)
#     $returnurl   : URL return field  (change only if found) (move case)
#     $url     : URL of the requested page
#     $header  : HTTP response header (make copy if need to modify)
#     $content : HTTP response HTML content (make copy if need to modify)
#    A rule can return a value if some rules uses another rule.
# 2) Write the type and help content (both return a string)
# 3) Write the rule itself
# 4) Add the rule to the rules data base (function "RulesDBmaker")
#    Consist of creating a new entry into the "RulesDB" hash table
#    which key is the nickname used for the rule and value is the address
#    of the function to be called.
#
# ! : Reserved nicknames : error move help all

# sub : Internal Error in function call 
sub iefc {
  my $func = shift @_;

  die "Internal Error : Function ($func) called from a wrong place\n";
}

## Error rules

# Case : "Page not found"
# nick : pnf
sub er_pnf {
  if ($op eq "type") {
    return ("error");
  } elsif ($op eq "help") {
    return ("Will check if web page title is not of a page not found type");
  } elsif ($op eq "check") {
    
    $returncode = $contentPNFerrorCode
      if ($header =~ m/^Title\:.*?(404|page\s+not\s+found)/m);

  } else { # Where did you call from ?
    &iefc("er_pnf");
  }
}


## Move rules

# Case : "We have|'ve moved to"
# nick : hm
sub mv_hm {
  if ($op eq "type") {
    return ("move");
  } elsif ($op eq "help") {
    return "Will check presence of \"We have|'ve moved to\" inside the page content";
  } elsif ($op eq "check") {
    my $val = $content;
    
    $val =~ s/\n//gm;
    
    $returnurl = $2
      if ($val =~ m%we\s+(have|\'ve)\s+moved\s+to.*?<a\s+href\=\"(.+?)\"%im);
  } else {
    &iefc("mv_hm");
  }
}

# Case : "this page has (been)? moved to"
# nick : hbm
sub mv_hbm {
  if ($op eq "type") {
    return ("move");
  } elsif ($op eq "help") {
    return "Will check presence of \"this page has (been)? moved to\" inside the page content";
  } elsif ($op eq "check") {
    my $val = $content;
    
    $val =~ s/\n//gm;
    
    $returnurl = $2
      if ($val =~ m%this\s+page\s+has(\s+been)?\s+moved\s+to.*?<a\s+href\=\"(.+?)\"%im);
  } else {
    &iefc("mv_hbm");
  }
}
