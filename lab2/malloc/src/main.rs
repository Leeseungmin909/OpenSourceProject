use std::io;

fn read_usize() -> usize {
    let mut input = String::new();
    io::stdin()
        .read_line(&mut input)
        .expect("입력을 읽지 못했습니다.");
    input
        .trim()
        .parse()
        .expect("유효한 숫자를 입력해주세요.")
}

fn read_matrix(rows: usize, cols: usize) -> Vec<Vec<i32>> {
    println!(
        "{}x{} 행렬의 원소를 입력합니다. 각 행은 {}개의 숫자를 공백으로 구분하여 입력하세요.",
        rows, cols, cols
    );

    let mut matrix: Vec<Vec<i32>> = Vec::with_capacity(rows);

    for i in 0..rows {
        println!("행 {} 입력:", i);
        let mut row_input = String::new();
        io::stdin()
            .read_line(&mut row_input)
            .expect("입력을 읽지 못했습니다.");

        let row: Vec<i32> = row_input
            .trim()
            .split_whitespace()
            .map(|s| s.parse().expect("숫자를 입력해주세요."))
            .collect();

        if row.len() != cols {
            panic!(
                "입력된 열의 개수({})가 지정된 개수({})와 다릅니다.",
                row.len(),
                cols
            );
        }

        matrix.push(row);
    }
    matrix
}

fn add_matrices(a: &Vec<Vec<i32>>, b: &Vec<Vec<i32>>) -> Vec<Vec<i32>> {
    a.iter()
        .zip(b.iter()) 
        .map(|(row_a, row_b)| {
            row_a
                .iter()
                .zip(row_b.iter())
                .map(|(val_a, val_b)| val_a + val_b) 
                .collect::<Vec<i32>>() 
        })
        .collect::<Vec<Vec<i32>>>() 
}

fn print_matrix(matrix: &Vec<Vec<i32>>) {
    for row in matrix {
        for val in row {
            print!("{}\t", val);
        }
        println!();
    }
}

fn main() {
    println!("행렬 A와 B의 크기 입력");
    println!("행 크기 입력:");
    let rows = read_usize();
    println!("열 크기 입력:");
    let cols = read_usize();

    println!("행렬 A 입력");
    let matrix_a = read_matrix(rows, cols);
    
    println!("행렬 B 입력");
    let matrix_b = read_matrix(rows, cols);

    let result_matrix = add_matrices(&matrix_a, &matrix_b);

    println!("덧셈 결과 (A + B)");
    print_matrix(&result_matrix);
}