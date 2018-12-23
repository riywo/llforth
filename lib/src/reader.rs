use std::collections::VecDeque;
use std::fs::File;
use std::io::{BufRead, BufReader};

use rustyline::error::ReadlineError;
use rustyline::{Editor, Config};
use atty::Stream;

pub enum Input {
    Word(String),
    Quote(String),
    Newline,
    Eof,
    Interrupted,
}

pub struct Reader {
    buffer: VecDeque<Input>,
    editor: Editor<()>,
}

impl Reader {
    pub fn new() -> Reader {
        let buffer = VecDeque::new();
        let editor = Editor::<()>::with_config(Config::builder()
            .build());
        Reader { buffer, editor }
    }

    pub fn read(&mut self) -> Input {
        if self.buffer.is_empty() {
            self.read_line()
        }
        match self.buffer.pop_front() {
            Some(input) => return input,
            None => return self.read(),
        }
    }

    pub fn read_file(&mut self, file: &str) {
        let file = File::open(file).expect("Can't open file");
        let file = BufReader::new(file);
        for line in file.lines() {
            match line {
                Ok(line) => {
                    self.process_line(line.trim());
                    self.buffer.push_back(Input::Newline);
                },
                Err(e) => panic!(e)
            }
        }
        self.buffer.push_back(Input::Eof);
    }

    fn read_line(&mut self) {
        let readline = self.editor.readline("> ");
        match readline {
            Ok(line) => {
                let line = line.trim();
                if ! atty::is(Stream::Stdin) {
                    print!("{}", line);
                }
                if line != "" {
                    self.editor.add_history_entry(line.as_ref());
                    self.process_line(line);
                }
                self.buffer.push_back(Input::Newline);
            },
            Err(ReadlineError::Eof) => {
                self.buffer.push_back(Input::Eof);
            },
            Err(ReadlineError::Interrupted) => {
                self.buffer.push_back(Input::Interrupted);
            },
            Err(err) => {
                panic!(err);
            },
        }
    }

    fn process_line(&mut self, line: &str) {
        let whitespace = line.find(char::is_whitespace);
        match whitespace {
            Some(index) => {
                let (word, next) = line.split_at(index);
                if word == "\\" {
                    return;
                } else if word == ".\"" {
                    self.buffer.push_back(Input::Word(String::from(word)));
                    let double_quote = next.find("\"");
                    match double_quote {
                        Some(index) => {
                            let (string, rest) = next.split_at(index);
                            self.buffer.push_back(Input::Quote(String::from(&string[1..])));
                            self.process_line(&rest[1..]);
                        },
                        None => {
                            self.buffer.push_back(Input::Quote(String::from(next)));
                            return;
                        },
                    }
                } else {
                    self.buffer.push_back(Input::Word(String::from(word)));
                    self.process_line(next.trim_left());
                }
            },
            None => {
                self.buffer.push_back(Input::Word(String::from(line)));
                return;
            },
        }
    }
}
