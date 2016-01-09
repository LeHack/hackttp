#!/usr/bin/perl -w
use strict; # {{{

use Date::Calc qw(Today_and_Now Now);
use CGI::Minimal;
use CGI::Cookie;
# }}}

sub main {
    my $cgi = CGI::Minimal->new();

    my $footer = sprintf 'At the moment it\'s %04d-%02d-%02d %02d:%02d:%02d, request method was: <b>%s</b>', Today_and_Now(), $ENV{REQUEST_METHOD};
    my $name = $cgi->param('name');
    my $output;
    if (not $name) {
        if ($name = fetch_from_cookie()) {
            $output = "Hey! I know you! You've been here before <u>$name</u>.<br/>Nice to see you again :)"
        }
        else {
            $output = "I'm very sorry, but I do not know your name.";
        }
    }
    else {
        my ($h) = Now();
        my $tod = "a morning";
        if ($h < 6 or $h > 19) {
            $tod = "an evening";
        }
        elsif ($h > 12) {
            $tod = "an afternoon";
        }
        $output = "Top of $tod to you $name! It is very nice to meet you :)";
    }

    # first print the cookie
    my $cookie;
    if ($name) {
        $cookie = bake_cookie($name);
    }
    # then the rest
    print_out($cookie, $output, $footer);

    return;
}

sub fetch_from_cookie {
    my %cookies = CGI::Cookie->parse($ENV{COOKIE});
    return ($cookies{name} ? $cookies{name}->value : undef);
}

sub bake_cookie {
    my ($name) = @_;
    return "Set-Cookie: name=$name\n";
}

sub print_out {
    my ($cookie, $output, $footer) = @_;

    $cookie ||= '';

    print << "EOF";
HTTP/1.x 200 OK
Content-Type: text/html; charset=UTF-8;
$cookie

<html>
    <head>
        <title>Created by form.pl</title>
        <link rel="stylesheet" type="text/css" href="/t/css/form.css">
    </head>
    <body>
        <h2>$output</h2>
        <p class="content">
            <img src="/t/images/hello.png" alt="hello"></img></br>
            <a href="/t/form.html">Back to the form</a>
        </p>
        <p class="footer">$footer</p>
    </body>
</html>
EOF

    return;
}

main();
