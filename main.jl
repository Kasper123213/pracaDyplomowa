
global randomNumbersIndex::Int = 1
global bestSolution::Vector{Int}
global bestLen::Int
global calcNewTemperature::Function
global currentTemperature::Float64

#obliczanie temperatury początkowej
function calcBeginTemperature(dimensions::Int, matrix::Matrix{Int})::Float64
    testSolution = copy(bestSolution)

    minLen, maxLen, testLen = 2147483647, 0, 0
    for _ in 1:100
        i = floor(Int, getRandomNumber(randomNumbersLen)*dimensions+1)
        j=i
        while j==i
            j = floor(Int, getRandomNumber(randomNumbersLen)*dimensions+1)
        end

        testLen += calcDiff(dimensions, matrix, testSolution, i, j)
        swapPoints!(i, j, testSolution)

        minLen = min(minLen, testLen)
        maxLen = max(maxLen, testLen)
    end
    return (maxLen - minLen) * 1.5

end


#wyliczamy różnice kosztów prze i po operacją swap
#funkcja powinna byc wykonywana przed operacją swap
@inline function calcDiff(dimensions::Int, matrix::Matrix{Int}, testSolution::Vector{Int}, indexI::Int, indexJ::Int)::Int
    diff = 0
    if indexI==1 || indexJ==1
        if max(indexI, indexJ)==2
            diff-=matrix[testSolution[1], testSolution[2]]
            diff-=matrix[testSolution[2], testSolution[3]]
            diff-=matrix[testSolution[dimensions], testSolution[dimensions+1]]

            diff+=matrix[testSolution[2], testSolution[1]]
            diff+=matrix[testSolution[1], testSolution[3]]
            diff+=matrix[testSolution[dimensions], testSolution[2]]
        elseif max(indexI, indexJ)==dimensions
            diff-=matrix[testSolution[1], testSolution[2]]
            diff-=matrix[testSolution[dimensions-1], testSolution[dimensions]]
            diff-=matrix[testSolution[dimensions], testSolution[dimensions+1]]

            diff+=matrix[testSolution[dimensions], testSolution[2]]
            diff+=matrix[testSolution[dimensions-1], testSolution[1]]
            diff+=matrix[testSolution[1], testSolution[dimensions]]
        else
            index = max(indexI, indexJ)
            diff-=matrix[testSolution[1], testSolution[2]]
            diff-=matrix[testSolution[index-1], testSolution[index]]
            diff-=matrix[testSolution[index], testSolution[index+1]]
            diff-=matrix[testSolution[dimensions], testSolution[dimensions+1]]

            diff+=matrix[testSolution[index], testSolution[2]]
            diff+=matrix[testSolution[index-1], testSolution[1]]
            diff+=matrix[testSolution[1], testSolution[index+1]]
            diff+=matrix[testSolution[dimensions], testSolution[index]]
        end

    elseif indexI+1==indexJ || indexJ+1==indexI
        index1 = min(indexI, indexJ)
        index2 = max(indexI, indexJ)

        diff-=matrix[testSolution[index1-1], testSolution[index1]]
        diff-=matrix[testSolution[index1], testSolution[index2]]
        diff-=matrix[testSolution[index2], testSolution[index2+1]]

        diff+=matrix[testSolution[index1-1], testSolution[index2]]
        diff+=matrix[testSolution[index2], testSolution[index1]]
        diff+=matrix[testSolution[index1], testSolution[index2+1]]
    else
        diff-=matrix[testSolution[indexJ-1], testSolution[indexJ]]
        diff-=matrix[testSolution[indexJ], testSolution[indexJ+1]]
        diff-=matrix[testSolution[indexI-1], testSolution[indexI]]
        diff-=matrix[testSolution[indexI], testSolution[indexI+1]]

        diff+=matrix[testSolution[indexI-1], testSolution[indexJ]]
        diff+=matrix[testSolution[indexJ], testSolution[indexI+1]]
        diff+=matrix[testSolution[indexJ-1], testSolution[indexI]]
        diff+=matrix[testSolution[indexI], testSolution[indexJ+1]]

    end
    return diff
end








#główna pętlan algorytmu symulowanego wyżarzania
function start(dimensions::Int, matrix::Matrix{Int}, startTime::Float64, expectedLen::Int)
    global currentTemperature, bestLen

    greedyAlgorithm(dimensions, matrix)
    beginningTemperature = calcBeginTemperature(dimensions, matrix)
    epochLen::Int = (dimensions * (dimensions - 1)) ÷ 2

    currentTemperature = beginningTemperature

    currentSolution = copy(bestSolution)
    currentLen = bestLen
    iteration = 0
    testLen = currentLen
    testSolution = copy(currentSolution)
    while currentTemperature >= 0.001 && bestLen >= expectedLen
        for _ in 1:epochLen
            i = floor(Int, getRandomNumber(randomNumbersLen)*dimensions+1)
            j=i
            while i == j
                j = floor(Int, getRandomNumber(randomNumbersLen)*dimensions+1)
            end

            testLenBefore = testLen
            testLen += calcDiff(dimensions, matrix, testSolution, i, j)
            #obliczamy różnice kosztów sąsiada i obecnego rozwiązania
            delta = testLen - currentLen

            if delta <= 0
                swapPoints!(i, j, testSolution)
                currentLen = testLen
                swapPoints!(i, j, currentSolution)
                if currentLen < bestLen
                    bestLen = currentLen
                    copyto!(bestSolution, currentSolution)
                end
            elseif getRandomNumber(randomNumbersLen) < exp(-delta / currentTemperature)
                swapPoints!(i, j, testSolution)
                currentLen = testLen
                swapPoints!(i, j, currentSolution)
            else
                testLen = testLenBefore
            end
        end
        iteration += 1
        currentTemperature = calcNewTemperature(currentTemperature, iteration)
    end
end


@inline function getRandomNumber(randomNumbersLen::Int)::Float64
    global randomNumbersIndex
    number = randomNumbers[randomNumbersIndex]/2147483647.0

    if randomNumbersIndex >= randomNumbersLen
        randomNumbersIndex = 1
    else
        randomNumbersIndex += 1
    end

    return number
end


# Swapowanie punktów - in-place
@inline function swapPoints!(i::Int, j::Int, testSolution::Vector{Int})
    testSolution[i], testSolution[j] = testSolution[j], testSolution[i]
     if i==1 || j==1
         testSolution[length(testSolution)] = testSolution[1]
      end
end




function readRandomNumbers(path::String)::Vector{Int}
    randomNumbersFile = open(path, "r")

    # Wczytanie całego pliku jako jednego dużego stringa
    # Podział zawartości pliku na liczby i ich parsowanie
    randomNumbers = parse.(Int, split(read(randomNumbersFile, String)))

    # Zamknięcie pliku
    close(randomNumbersFile)

    return randomNumbers
end


function readMatrix(matrixPath::SubString{String})
    matrix = nothing
    dimensions = nothing


    matrixFile = open(matrixPath, "r")

    # Wczytaj wymiary macierzy
    while !eof(matrixFile)
        line = readline(matrixFile)
        if occursin("DIMENSION:", line)
            dimensions = parse(Int, filter(c -> isdigit(c), line))
            matrix = Array{Int}(undef, dimensions, dimensions)
            break
        end
    end


    while !eof(matrixFile)
        line = readline(matrixFile)
        if occursin("EDGE_WEIGHT_SECTION", line)
            break
        end
    end

    row, col = 1, 1
    while !eof(matrixFile)
        line = readline(matrixFile)
        if !occursin("EOF", line)
            for i in split(line)
                matrix[row, col] = parse(Int, i)
                col += 1
                if col > dimensions
                    col = 1
                    row += 1
                end
            end
        end
    end
    close(matrixFile)
    return dimensions, matrix
end

function greedyAlgorithm(dimensions::Int, matrix::Matrix{Int})
    global bestLen = 0
    currentCity = 1
    while length(bestSolution) != dimensions
        possible_cities = [i for i in 1:dimensions if i ∉ bestSolution]
        nextCity = possible_cities[argmin([matrix[currentCity, i] for i in possible_cities])]
        push!(bestSolution, nextCity)
        bestLen += matrix[currentCity, nextCity]
        currentCity = nextCity
    end

    push!(bestSolution, bestSolution[1])  # Powrót do początku
    bestLen += matrix[currentCity, 1]  # Dodaj powrót do długości


end


println("Autor: Kasper Radom 264023")


# Ścieżka do pliku konfiguracyjnego
println("Podaj ścieżkę do pliku konfiguracyjnego")
print(">>")
initFilePath = readline()
println()


# Otwórz plik konfiguracyjny
initFile = open(initFilePath, "r")

global const randomNumbers::Vector{Int} = readRandomNumbers(readline(initFile))
global randomNumbersLen = length(randomNumbers)

# Wczytaj macierz
while !eof(initFile)
    initLine = readline(initFile)
    #pominięcie zakomentowanych wierszy
    if length(initLine) == 0 || initLine[1] == '#' #todo
        continue
    end

    println(initLine)
    println()
    println()

    words = split(initLine)

    dimensions::Int, matrix::Matrix{Int} = readMatrix(words[1])

    # Wczytywanie parametrów z pliku konfiguracyjnego
    expectedLen = parse(Int, words[2])# oczekiwany koszt rozwiązania
    #oczekiwane rozwiązanie jest akceptowane gdy jest co najwyżej o 1% dorsze od oczekiwanego
    expectedLen = Int(floor(expectedLen*1.01))

    coolingFactor = parse(Float64, words[3])
    coolingType = parse(Int, words[4])
    global randomNumbersIndex = parse(Int, words[5]) + 1
    if randomNumbersIndex > randomNumbersLen
        global randomNumbersIndex = 1
    end



    # Zainicjuj początkową ścieżkę (algorytm zachłanny)
    global bestSolution::Vector{Int} = [1]  # Zaczynamy od miasta 1


    # deklarowanie funkcji do zmiany temperatury
    if coolingType == 1  # Geometryczne
        global calcNewTemperature  = (temperature , iteration) -> temperature * coolingFactor
    elseif coolingType == 2  # Logarytmiczne
        global calcNewTemperature  = (temperature , iteration) -> temperature / (1 + coolingFactor * log(iteration + 1))
    else # Wykładnicze
        global calcNewTemperature  = (temperature , iteration) -> temperature * exp(-coolingFactor * iteration)
    end




    # Mierzenie czasu
    startTime = Float64(time())


    timing = @timed start(dimensions, matrix, startTime, expectedLen)


    result = string(bestLen, ";", timing.time, "\n")

    # Dopisanie tekstu do pliku
    open("wynikiJulia.csv", "a") do resultFile
        write(resultFile, result)
    end




end


