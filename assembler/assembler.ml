open Str

let rtype = ["add";"sub";"sll";"slt";"sltu";"xor";"srl";"or";"and"]
let mtype = ["mul";"mulh";"mulhsu";"mulhu";"div";"divu";"rem";"remu"]
let itype = ["addi";"slli";"slti";"sltiu";"xori";"srli";"srai";"ori"]
let ltype = ["lb";"lh";"lw";"lbu";"lhu"]
let stype = ["sb";"sh";"sw"]
let btype = ["beq";"bne";"blt";"bge";"bltu";"bgeu";]
let jalr = ["jalr"]
let jal = ["jal"]


let contents = In_channel.with_open_text "test1.txt" In_channel.input_all

let testinstruction = "add rd, rs1, rs2"

let filesplitter input = String.split_on_char '\n' input

let commstrip input = List.map ( fun x -> match String.split_on_char '#' x with
  | [] -> ""
  | h :: _ -> h 
  ) input 

let linesplitter input = List.map ( fun x -> Str.split(Str.regexp "[ ,\r]+") x) input

let stripempty input = List.filter (fun x -> x <> []) input

let increment = ref 0

let acc () = increment := !increment + 4

let symbols = Hashtbl.create 10

let insertsymbol input = Hashtbl.replace symbols input !increment

let run1 input = List.iter ( fun x-> 
  match x with
  | [] -> ()
  | h :: t -> 
    if String.contains h ':' then begin 
      insertsymbol (List.hd (String.split_on_char ':' h));
    if t <> [] then acc ()
    end else acc()
) input

let parsed = contents |> filesplitter |> commstrip |> linesplitter |> stripempty 

let () = run1 parsed

let () = Hashtbl.iter (fun k v -> Printf.printf "%s -> %d\n" k v) symbols
