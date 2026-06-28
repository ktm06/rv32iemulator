open Str

let contents = In_channel.with_open_text "instr.txt" In_channel.input_all

let testinstruction = "add rd, rs1, rs2"

let filesplitter input = String.split_on_char '\n' input
let linesplitter input = List.map ( fun x -> Str.split(Str.regexp "[ ,]+") x) input

let parsed = contents |> filesplitter |> linesplitter