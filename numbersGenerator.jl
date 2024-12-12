using Random
using DelimitedFiles
println("Podaj ile ma być liczb: ")
input = parse(Float64, readline())
# Generowanie 100 losowych liczb całkowitych z przedziału od 1 do typemax(Int)
randomNumbers= [rand(1:typemax(Int32)) for _ in 1:input]
# Zapisanie listy do pliku
writedlm("randomNumbers.txt", randomNumbers)