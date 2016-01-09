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

    # prepare the cookie
    my $cookie = '';
    if ($cgi->param('reset')) {
        $cookie = drop_cookie();
        $output = "Goodbye $name :)";
    }
    else {
        $cookie = ($name ? bake_cookie($name) : '');
    }

    # then the rest
    printf get_template(), $cookie, $output, $footer;

    return;
}

sub fetch_from_cookie {
    my %cookies = CGI::Cookie->parse($ENV{COOKIE});
    return ($cookies{name} ? $cookies{name}->value : undef);
}

sub bake_cookie {
    my ($name) = @_;
    return "Set-Cookie: name=$name; HttpOnly";
}

sub drop_cookie {
    my ($name) = @_;
    return bake_cookie("deleted") . "; MaxAge=0; expires=Thu, 01 Jan 1970 00:00:00 GMT";
}

sub get_template {
    my $template = << "EOF";
HTTP/1.x 200 OK
Content-Type: text/html; charset=UTF-8;
%s

<html>
    <head>
        <title>Created by form.pl</title>
        <link rel="stylesheet" type="text/css" href="/t/css/form.css">
    </head>
    <body>
        <h2>%s</h2>
        <p class="content">
            <img src="/t/images/hello.png" alt="hello"></img></br>
            <a href="/t/form.html">Back to the form</a><br/>
            <a href="/t/cgi/form.pl">Show with no input params</a><br/>
            <a href="/t/cgi/form.pl?reset=1">Drop cookie</a>
        </p>
        <p class="footer">%s</p>
    </body>
</html>
EOF

    return $template;
}

main();
