#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2025 Richard Liebscher <r1tschy@posteo.de>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import argparse
import sys
import re
import locale
import datetime
import textwrap
import subprocess

from pathlib import Path
from typing import Callable, Match, NamedTuple


HEADER_RE = re.compile(
    r"^\*\s+(?P<date>\w+\s+\w+\s+\d+\s+\d+)\s+(?P<author>[^<]+)\s+<(?P<email>[^>]+)>\s+(?P<version>[\w.~]+)-(?P<release>\d+)",
    re.M,
)

def git_user_name():
    return subprocess.check_output(["git", "config", "user.name"]).strip().decode("utf-8")

def git_user_email():
    return subprocess.check_output(["git", "config", "user.email"]).strip().decode("utf-8")


class ChangelogHeader:
    def __init__(self, match: Match[str]):
        self.match = match

    @property
    def date(self):
        return self.match.group("date")

    @property
    def author(self):
        return self.match.group("author")

    @property
    def email(self):
        return self.match.group("email")

    @property
    def version(self):
        return self.match.group("version")

    @property
    def release(self):
        return self.match.group("release")

    @property
    def date_loc(self):
        return (self.match.start("date"), self.match.end("date"))

    @property
    def author_loc(self):
        return (self.match.start("author"), self.match.end("author"))

    @property
    def email_loc(self):
        return (self.match.start("email"), self.match.end("email"))

    @property
    def version_loc(self):
        return (self.match.start("version"), self.match.end("version"))

    @property
    def release_loc(self):
        return (self.match.start("release"), self.match.end("release"))

    @property
    def loc(self):
        return (self.match.start(), self.match.end())


class Change(NamedTuple):
    start: int
    end: int
    replacement: str



def find_first_header(changelog: str) -> ChangelogHeader:
    match = HEADER_RE.search(changelog)
    if not match:
        raise RuntimeError("No changelog entry found. Right format?")

    return ChangelogHeader(match)


def format_datetime(datetime: datetime.datetime):
    locale.setlocale(locale.LC_ALL, "C")
    return datetime.strftime("%a %b %d %Y")


def get_changelog() -> Path:
    changelogs = list((Path(__file__).parent.parent / "rpm").glob("*.changes"))
    if len(changelogs) == 0:
        raise RuntimeError("No changelog file found")
    elif len(changelogs) != 1:
        raise RuntimeError("More than one changelog file found")
    return changelogs[0]


def apply_commandfn(fn: Callable[[str, list[str]], Change], cmd_args: list[str]):
    changelog_path = get_changelog()
    with changelog_path.open("r", encoding="utf-8") as fp:
        changelog = fp.read()

    change = fn(changelog, cmd_args)

    if change:
        with changelog_path.open("w", encoding="utf-8") as fp:
            fp.write(changelog[:change.start])
            fp.write(change.replacement)
            fp.write(changelog[change.end:])


def release(changelog: str, cmd_args: list[str]) -> Change:
    header = find_first_header(changelog)
    return Change(
        start=header.date_loc[0],
        end=header.date_loc[1],
        replacement=format_datetime(datetime.datetime.now())
    )

def prepare(changelog: str, cmd_args: list[str]) -> Change:
    argparser = argparse.ArgumentParser(description="Add entry")
    argparser.add_argument("version")
    args = argparser.parse_args(cmd_args)

    date = format_datetime(datetime.datetime.now())
    name = git_user_name()
    email = git_user_email()

    template = textwrap.dedent(f"""
        * {date} {name} <{email}> {args.version}-0
        - 

    """)

    header = find_first_header(changelog)
    return Change(
        start=header.loc[0],
        end=header.loc[0],
        replacement=template
    )


COMMANDS: dict[str, Callable[[str, list[str]], Change]] = {
    "release": release,
    "prepare": prepare,
}


def main():
    argparser = argparse.ArgumentParser(description="Edit changelog")
    argparser.add_argument("command", choices=COMMANDS.keys(), help="One of " + ", ".join(COMMANDS.keys()))
    argparser.add_argument("args", nargs=argparse.REMAINDER)
    args = argparser.parse_args()

    commandfn = COMMANDS.get(args.command)
    if commandfn is None:
        print(
            "Unknown command: "
            + args.command
            + ". Use one of "
            + ", ".join(COMMANDS.keys())
            + ".",
            file=sys.stderr,
        )
        sys.exit(1)

    apply_commandfn(commandfn, args.args)



if __name__ == "__main__":
    main()
