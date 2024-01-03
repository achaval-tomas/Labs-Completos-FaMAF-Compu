-- ejercicio 1.a)
esCero :: Int -> Bool
esCero x = x == 0     -- Verifica si un entero x es 0.

-- ejercicio 1.b)
esPositivo :: Int -> Bool
esPositivo x = x > 0      -- Verifica si un entero x es positivo.

-- ejercicio 1.c)
esVocal :: Char -> Bool
esVocal x = elem x ['a', 'e', 'i', 'o', 'u'] -- Verifica si el caracter x es una vocal.

-- ejercicio 2.a)
paratodo :: [Bool] -> Bool
paratodo [] = True
paratodo (x:xs) = x && paratodo xs   -- Verfica que todos los elementos x de la lista x:xs sean True.

-- ejercicio 2.b)
sumatoria :: [Int] -> Int
sumatoria [] = 0
sumatoria (x:xs) = x + sumatoria xs -- Suma todos los enteros x de la lista x:xs.

-- ejercicio 2.c)
productoria :: [Int] -> Int
productoria [] = 1
productoria (x:xs) = x * (productoria xs)  -- Multiplica todos los enteros x de la lista x:xs.

-- ejercicio 2.d)
factorial :: Int -> Int
factorial 0 = 1
factorial n = n * (factorial (n-1)) -- Devuelve el factorial de cualquier numero entero (positivo).

-- ejercicio 2.e)
promedio :: [Int] -> Int
promedio [] = 0
promedio xs = (sumatoria xs) `div` (length xs)  -- Calcula el promedio entre los valores enteros de una lista.

-- ejercicio 3)
pertenece :: Int -> [Int] -> Bool
pertenece x [] = False
pertenece a (x:xs) = a == x || pertenece a xs -- Chequea si el elemento a pertenece a la lista x:xs.

-- ejercicio 4.a)
paratodo' :: [a] -> (a -> Bool) -> Bool
paratodo' [] t = True
paratodo' (x:xs) t = (t x) && paratodo' xs t  -- Verifica que todo elemento x de la lista x:xs satisfaga el predicado t. 

-- ejercicio 4.b)
existe' :: [a] -> (a -> Bool) -> Bool
existe' [] t = False
existe' (x:xs) t = (t x) || existe' xs t  -- Chequea si algun elemento x de la lista x:xs satisface el predicado t.

-- ejercicio 4.c)
sumatoria' :: [a] -> (a -> Int) -> Int
sumatoria' [] t = 0
sumatoria' (x:xs) t = (t x) + sumatoria' xs t  -- Suma los enteros x de la lista x:xs tras aplicarles una funcion t.

-- ejercicio 4.d)
productoria' :: [a] -> (a -> Int) -> Int
productoria' [] t = 1
productoria' (x:xs) t = (t x) * productoria' xs t  -- Multiplica los enteros x de la lista x:xs tras aplicarles una funcion t.

-- ejercicio 5)
paratodo'' :: [Bool] -> Bool
paratodo'' xs = paratodo' xs id -- Redefino paratodo (2.a) utilizando paratodo' (4.a).

-- ejercicio 6.a)
todosPares :: [Int] -> Bool
todosPares xs = paratodo' xs even  -- Verifico que todos los elementos de una lista sean enteros pares.

-- ejercicio 6.b)
esMultiplo :: Int -> Int -> Bool
esMultiplo x y = mod x y == 0 -- Defino una funcion que determina si un numero (x) es multiplo de otro (y).

hayMultiplo :: Int -> [Int] -> Bool
hayMultiplo x xs = existe' xs (`esMultiplo` x) -- Verifico si existe en xs un multiplo de x utilizando existe'.

-- ejercicio 6.c) 
sumaCuadrados :: Int -> Int
sumaCuadrados x = sumatoria' [0..x] (^2)  -- Suma los primeros x enteros cuadrados.

-- ejercicio 6.d)
factorial' :: Int -> Int
factorial' x = productoria' [1..x] (*1)  -- Redefino el factorial de x (2.d) sin utilizar recursion.

-- ejercicio 6.e)
dejaPares :: [Int] -> [Int]                    -- Defino una funcion que elimina los enteros impares de una lista
dejaPares [] = []                              -- para evitar usar filter (por ej. 9.c)).
dejaPares (x:xs) | even x = x:(dejaPares xs)
                 | otherwise = dejaPares xs

multiplicaPares :: [Int] -> Int                        -- Multiplico los enteros pares de la lista xs utilizando las funciones
multiplicaPares xs = productoria' (dejaPares xs) id    -- previamente creadas productoria' y dejaPares.

{- ejercicio 7)
    La funcion map toma una lista y una funcion, y devuelve la lista resultante
de aplicar la funcion a cada uno de sus elementos.
    La funcion filter toma una lista y un predicado (funcion que devuelve Bool),
y devuelve una lista con los elementos que satisfagan el predicado.
    La expresión map succ [1, -4, 6, 2, -8], es equivalente a la lista
[2, -3, 7, 3, -7], pues le suma 1 a cada elemento de la lista original.
    La expresión filter esPositivo [1, -4, 6, 2, -8] equivale a la lista [1, 6, 2]
pues solo se mantienen los elementos que satisfacen el predicado "esPositivo".
-}

-- ejercicio 8.a)
doblesDe :: Num a => [a] -> [a]
doblesDe [] = []
doblesDe (x:xs) = (x*2):(doblesDe xs)      -- Recursiva.

-- ejercicio 8.b)
duplicate :: Num a => [a] -> [a]
duplicate xs = map (*2) xs                 -- Mapeada.

-- ejercicio 9.a)
soloPares :: [Int] -> [Int]
soloPares [] = []
soloPares (p:ps) | even p = p:(soloPares ps)  -- Recursiva
                 | odd p = soloPares ps       -- por casos.

-- ejercicio 9.b)
onlyEven :: [Int] -> [Int]
onlyEven xs = filter even xs  -- Usando filter.

-- ejercicio 9.c)
multiPares :: [Int] -> Int -- Mejora del 6.e)
multiPares xs = productoria' (filter even xs) (*1) 

-- ejercicio 10.a)
primIgualesA :: Eq a => a -> [a] -> [a]
primIgualesA v [] = []
primIgualesA v (x:xs) | v == x = v:(primIgualesA v xs) -- Recursion y
                      | otherwise = []                 -- casos.

-- ejercicio 10.b)
tramoInicialDe :: Eq a => a -> [a] -> [a]
tramoInicialDe v xs = takeWhile (==v) xs   -- Utilizo takeWhile.

-- ejercicio 11.a)
primIguales :: Eq a => [a] -> [a]
primIguales [] = []
primIguales [a] = [a]
primIguales (x:(y:xs)) | x == y = x:(primIguales (y:xs))  -- Recursion
                       | otherwise = x:[]

-- ejercicio 11.b)
primIguales' :: Eq a => [a] -> [a]
primIguales' (x:xs) = primIgualesA x (x:xs) -- Utilizo primIgualesA.

-- ejercicio 12)
cuantGen :: (b -> b -> b) -> b -> [a] -> (a -> b) -> b
cuantGen op z [] t = z
cuantGen op z (x:xs) t = op (t x) (cuantGen op z xs t)  -- Defino funcion general

paraTodo :: [a] -> (a -> Bool) -> Bool
paraTodo xs p = cuantGen (&&) True xs p  -- redefino 4.a)

existE :: [a] -> (a -> Bool) -> Bool
existE xs p = cuantGen (||) False xs p   -- redefino 4.b)

sumaToria :: [a] -> (a -> Int) -> Int
sumaToria xs p = cuantGen (+) 0 xs p     -- redefino 4.c)

producToria :: [a] -> (a -> Int) -> Int
producToria xs p = cuantGen (*) 1 xs p   -- redefino 4.d)

{- ejercicio 13
a) Esta bien tipado, (x, y) es de tipo (a, b), x tiene tipo a, e y tiene tipo b.
b) No esta bien escrito pues f toma una lista de tuplas.
c) Si esta bien tipado aunque se podria utilizar pattern matching para mejor uso de las tuplas x es del tipo (a, b).
d) Esta bien tipado, x tipo a e y tipo b.
e) Esta bien tipado pero no cubre todos los casos.
f) Esta bien tipado pero no cubre todos los casos ((x, 1):xs) es de tipo [(Int, a)] al igual que xs, y x es de tipo Int.
g) Esta bien tipado, a es de tipo (Int -> Int) y b de tipo Int.
h) Esta bien tipado pero no cubre todos los casos, a es de tipo (Int -> Int).
i) Esta mal tipado pues la funcion solo toma 2 argumentos y uno de ellos debe ser una funcion (Int -> Int).
-}

-- ejercicio 14.a)
fa :: (a, b) -> b
fa (x, y) = y

-- ejercicio 14.b)
-- fb :: (a, b) -> c
-- NO SE PUEDE

-- ejercicio 14.c)
fc :: (a -> b) -> a -> b
fc y x = y x

-- ejercicio 14.d)
fd :: (a -> b) -> [a] -> [b]
fd x [] = []
fd x (y:ys) = (x y):(fd x ys)

-- ejercicio 14.e)
fe :: (a -> b) -> (b -> c) -> a -> c
fe a b c = b (a c)

-- Funciones inventadas
base10aBinario :: Int -> [Int]
base10aBinario 1 = [1]
base10aBinario 0 = [0]
base10aBinario x = (base10aBinario (div x 2)) ++ [mod x 2]
