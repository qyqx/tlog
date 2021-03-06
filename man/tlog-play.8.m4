m4_include(`man.m4')m4_dnl
m4_define(`M4_PROG_NAME', `play')m4_dnl
.\" Process this file with
.\" groff -man -Tascii tlog-play.8
m4_generated_warning(`.\" ')m4_dnl
.\"
.\" Copyright (C) 2016 Red Hat
.\"
.\" This file is part of tlog.
.\"
.\" Tlog is free software; you can redistribute it and/or modify
.\" it under the terms of the GNU General Public License as published by
.\" the Free Software Foundation; either version 2 of the License, or
.\" (at your option) any later version.
.\"
.\" Tlog is distributed in the hope that it will be useful,
.\" but WITHOUT ANY WARRANTY; without even the implied warranty of
.\" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\" GNU General Public License for more details.
.\"
.\" You should have received a copy of the GNU General Public License
.\" along with tlog; if not, write to the Free Software
.\" Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
.\"
.TH tlog-play "8" "March 2016" "Tlog"
.SH NAME
tlog-play \- play back terminal I/O recorded by tlog-rec(8)

.SH SYNOPSIS
.B tlog-play
[OPTION...]

.SH DESCRIPTION
.B Tlog-play
is a playback program for terminal I/O recorded with tlog-rec(8).  It
reproduces the recording on the terminal it's run under, and can't change its
size, so the playback terminal size needs to match the recorded terminal size
for proper playback.

.B Tlog-play
loads its parameters from the system-wide configuration file M4_CONF_PATH(),
which can be overridden with command-line options described below.

.SH OPTIONS
M4_MAN_OPTS()

.SH FILES
.TP
M4_CONF_PATH()
The system-wide configuration file

.SH EXAMPLES
.TP
Play back contents of a file written with tlog-rec's "file" writer:
.B tlog-play -r file --file-path=recording.log

.TP
Play back a recording from ElasticSearch:
.B tlog-play -r es --es-baseurl=http://localhost:9200/tlog/tlog/_search --es-query=session:121

.SH SEE ALSO
tlog-play.conf(5), tlog-rec(8)

.SH AUTHOR
Nikolai Kondrashov <spbnick@gmail.com>
