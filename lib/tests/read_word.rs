extern crate assert_cmd;

use std::process::Command;
use assert_cmd::prelude::*;

fn run(stdin: &str, expected: &'static str) {
    let mut cmd = Command::cargo_example("read_word").unwrap();
    cmd
        .with_stdin()
        .buffer(stdin)
        .unwrap()
        .assert()
        .stdout(expected);
}

#[test]
fn example_0() {
    run("", "");
}

#[test]
fn example_1() {
    run("foo", "foo  ok [(\"foo\", 3)]\n");
}

#[test]
fn example_2() {
    run("foo bar", "foo bar  ok [(\"foo\", 3), (\"bar\", 3)]\n");
}
