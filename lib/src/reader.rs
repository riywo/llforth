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
    editor: Editor<()>,
    buffer: VecDeque<Input>,
}

impl Reader {
    pub fn new() -> Reader {
        let config = Config::builder()
            .build();
        Reader {
            editor: Editor::<()>::with_config(config),
            buffer: VecDeque::new(),
        }
    }

    pub fn set_file(&mut self, file_name: &str) {
//        match File::open(file_name) {
//            Ok(file) => self.file = Some(file),
//            Err(err) => panic!(err),
//        }
    }

    pub fn read(&mut self) -> Input {
//        if self.buffer.is_empty() {
//            if self.file.is_some() {
//                self.read_file();
//            } else {
//                self.read_line();
//            }
//        }
        if self.buffer.is_empty() {
            self.read_line();
        }
        match self.buffer.pop_front() {
            Some(input) => return input,
            None => return self.read(),
        }
    }

    fn read_file(&mut self) {
//        let file = self.file.unwrap();
//        let f = BufReader::new(file);
//        for line in f.lines() {
//        }
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
