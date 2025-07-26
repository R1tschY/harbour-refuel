import argparse
import subprocess
import sys
import re
from pathlib import Path
from typing import Callable

def mod_content(path: Path, fn: Callable[[str], str]):
    content = path.read_text(encoding="utf-8")
    new_content = fn(content)
    path.write_text(new_content, encoding="utf-8")


def exec_to_text(args: list[str]) -> str:
    return subprocess.run(args, check=True, encoding="utf-8").stdout


def mod_rpm_version(rpm_release: str) -> Callable[[str], str]:
    def fn(content: str) -> str:
        return re.sub(r"^Version:\s+(.*)$", rpm_release, content, 1)
    return fn


def main():
    argparser = argparse.ArgumentParser(description="Do a release")
    argparser.add_argument("version")
    args = argparser.parse_args()

    version_str = args.version
    version_tuple = tuple(map(int, version_str.split(".")))
    major, minor, _ = version_tuple
    rpm_release = f"{version_str}-1"
    rpm_root = Path(__file__).parent.parent / "rpm"
    spec_path = next(rpm_root.glob("*.spec"))
    changes_path = next(rpm_root.glob("*.changes"))
    next_version = major, minor + 1, 0
    next_version_str = ".".join(map(str, next_version))
    next_rpm_version = f"{next_version_str}-0"

    # create release commit
    if rpm_release not in changes_path.read_text(encoding="utf-8"):
        raise RuntimeError("Changelog missing")

    subprocess.check_call([sys.executable, str(Path(__file__).parent / "changelog.py"), "release"])
    mod_content(spec_path, mod_rpm_version(rpm_release))
    subprocess.check_call(["git", "commit", "-m", f"Release {rpm_release}", str(spec_path), str(changes_path)])
    subprocess.check_call(["git", "tag", "-a", f"Release {version_str}", f"v{version_str}"])
    subprocess.check_call(["git", "push", exec_to_text(["git", "branch", "--show-current"]), f"v{version_str}"])

    # create dev version
    subprocess.check_call([sys.executable, str(Path(__file__).parent / "changelog.py"), "prepare", f"{next_version_str}-1"])
    mod_content(spec_path, mod_rpm_version(next_version_str))
    subprocess.check_call(["git", "commit", "-m", f"Increment version to {next_rpm_version}", str(spec_path), str(changes_path)])
    


if __name__ == "__main__":
    main()