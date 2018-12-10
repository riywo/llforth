use std::collections::VecDeque;

use rustyline::error::ReadlineError;
use rustyline::{Editor, Config};

use atty::Stream;

pub struct Prompt {
    editor: Editor<()>,
    buffer: VecDeque<Input>,
}

pub enum Input {
    Word(String),
    Enter,
    Eof,
    Interrupted,
}

impl Prompt {
    pub fn new() -> Prompt {
        let config = Config::builder()
            .build();
        Prompt {
            editor: Editor::<()>::with_config(config),
            buffer: VecDeque::new(),
        }
    }

    pub fn read(&mut self) -> Input {
        if self.buffer.is_empty() {
            self.read_line();
        }
        match self.buffer.pop_front() {
            Some(input) => return input,
            None => return self.read(),
        }
    }

    fn read_line(&mut self) {
        let readline = self.editor.readline("> ");
        match readline {
            Ok(line) => {
                let line = line.trim_end();
                if ! atty::is(Stream::Stdin) {
                    print!("{}", line);
                }
                self.editor.add_history_entry(line.as_ref());
                for word in line.split(" ") {
                    let input = Input::Word(String::from(word));
                    self.buffer.push_back(input);
                }
                self.buffer.push_back(Input::Enter);
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
}