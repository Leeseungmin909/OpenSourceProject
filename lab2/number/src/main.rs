use std::io;
use std::collections::HashMap;

fn read_string() -> String {
    let mut input = String::new();
    io::stdin()
        .read_line(&mut input)
        .expect("입력을 읽지 못했습니다.");
    input.trim().to_string()
}

fn main() {
    let mut phonebook: HashMap<String, String> = HashMap::new();

    loop {
        println!("--- 전화번호부 ---");
        println!("1: 추가/수정");
        println!("2: 검색");
        println!("3: 전체 보기");
        println!("4: 종료");
        println!("-----------------");
        println!("메뉴를 선택하세요:");

        let choice = read_string();

        match choice.as_str() {
            "1" => {
                println!("이름을 입력하세요:");
                let name = read_string();
                println!("전화번호를 입력하세요:");
                let phone = read_string();
                
                phonebook.insert(name.clone(), phone);
                println!("'{}'의 정보가 저장되었습니다.", name);
            }
            "2" => {
                println!("검색할 이름을 입력하세요:");
                let name = read_string();
                
                match phonebook.get(&name) {
                    Some(phone) => {
                        println!("이름: {}, 전화번호: {}", name, phone);
                    }
                    None => {
                        println!("'{}'의 정보를 찾을 수 없습니다.", name);
                    }
                }
            }
            "3" => {
                if phonebook.is_empty() {
                    println!("전화번호부가 비어있습니다.");
                } else {
                    println!("\n--- 전체 목록 ---");
                    for (name, phone) in &phonebook {
                        println!("이름: {}, 전화번호: {}", name, phone);
                    }
                }
            }
            "4" => {
                println!("프로그램을 종료합니다.");
                break;
            }
            _ => {
                println!("잘못된 입력입니다. 1~4 사이의 숫자를 입력하세요.");
            }
        }
    }
}