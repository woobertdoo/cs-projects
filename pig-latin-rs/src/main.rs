struct Word {
    english: String,
    pig_latin: String,
}

fn split_sentence(words: &str) -> Vec<&str> {
    let mut temp_str: String = String::new();
    let mut word_vec: Vec<&str> = vec![];
    let mut prev_ch: char = 'a';

    for i in 0..words.len() {
        let ch: char = words.as_bytes()[i] as char;
        if ch.is_alphabetic() {
            temp_str = temp_str + ch.to_string().as_str();
        } else if i > 0 && ch.is_whitespace() && !prev_ch.is_whitespace() {
            word_vec.push(&temp_str.as_str());
            temp_str = String::new();
        }
    }

    word_vec
}

fn main() {
    let message = "hello";
}
