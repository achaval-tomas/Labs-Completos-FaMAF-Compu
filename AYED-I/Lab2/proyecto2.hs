-- ejercicio 1 a)
data Carrera = Matematica | Fisica | Computacion | Astronomia deriving (Eq, Show) -- Defino el tipo Carrera

-- ejercicio 1 b)
titulo :: Carrera -> String                                         -- Funcion que devuelve el nombre
titulo Matematica = "Licenciatura en Matemática"                    -- de las carreras del tipo Carrera.
titulo Fisica = "Licenciatura en Física"
titulo Computacion = "Licenciatura en Ciencias de la Computación"
titulo Astronomia = "Licenciatura en Astronomía"

-- ejercicio 1 c) + 2 a)
data NotaBasica = Do | Re | Mi | Fa | Sol | La | Si deriving (Eq, Ord, Show) -- Defino el tipo NotaBasica

-- ejercicio 1 d)
cifradoAmericano :: NotaBasica -> Char  -- Funcion que asocia las notas basicas con el
cifradoAmericano Do = 'C'               -- cifrado utilizado en EEUU.
cifradoAmericano Re = 'D'
cifradoAmericano Mi = 'E'
cifradoAmericano Fa = 'F'
cifradoAmericano Sol = 'G'
cifradoAmericano La = 'A'
cifradoAmericano Si = 'B'

-- ejercicio 3. a)
minimoElemento :: Ord a => [a] -> a
minimoElemento [a] = a
minimoElemento (a:as) = min a (minimoElemento as)

-- ejercicio 3. b)
minimoElemento' :: (Bounded a, Ord a) => [a] -> a
minimoElemento' [] = minBound
minimoElemento' (x:xs) = minimoElemento (x:xs)

-- ejercicio 4. a)
type Ingreso = Int
data Cargo = Titular | Asociado | Adjunto | Asistente | Auxiliar deriving (Eq, Show)
data Area = Administrativa | Ensenanza | Economica | Postgrado deriving (Eq, Show)
data Persona = Decane | Docente Cargo | NoDocente Area | Estudiante Carrera Ingreso deriving (Eq, Show)

-- b) El constructor Docente es de tipo Persona.

-- c)
cuantos_doc :: [Persona] -> Cargo -> Int
cuantos_doc [] p = 0
cuantos_doc ((Docente b):xs) c | b == c = 1 + (cuantos_doc xs c)
                               | otherwise = 0 + (cuantos_doc xs c)
cuantos_doc (x:xs) c = cuantos_doc xs c

-- d)
cuantosdoc :: [Persona] -> Cargo -> Int
cuantosdoc xs p = length (filter (== Docente p) xs)

-- ejercicio 5)
-- a)
data Alteracion = Bemol | Sostenido | Natural deriving (Eq, Show)
data NotaMusical = Nota NotaBasica Alteracion deriving Show

sonido :: NotaBasica -> Int
sonido Do = 1
sonido Re = 3
sonido Mi = 5
sonido Fa = 6
sonido Sol = 8
sonido La = 10
sonido Si = 12

-- b)
sonidoCromatico :: NotaMusical -> Int
sonidoCromatico (Nota a b) | b == Sostenido = 1 + sonido a
                           | b == Bemol = (-1) + sonido a
                           | b == Natural = sonido a

-- c) 
instance Eq NotaMusical where
    m == n = ((mod (sonidoCromatico m) 12) == (mod (sonidoCromatico n) 12))

-- d)
instance Ord NotaMusical where
    m <= n = (sonidoCromatico m <= sonidoCromatico n)

-- ejercicio 6) 
primerElemento :: [a] -> Maybe a
primerElemento [] = Nothing
primerElemento (t:ts) = Just t

-- ejercicio 7.a.1) 
data Cola = VaciaC | Encolada Persona Cola deriving Show

atender :: Cola -> Maybe Cola
atender VaciaC = Nothing
atender (Encolada p c) = Just c

-- ejercicio 7.a.2)
encolar :: Persona -> Cola -> Cola
encolar p c = Encolada p c

-- ejercicio 7.a.3)
busca :: Cola -> Cargo -> Maybe Persona
busca VaciaC r = Nothing
busca (Encolada p c) r | p == Docente r = Just (Docente r)
                       | otherwise = busca c r

-- ejercicio 7.b) Cola posee un aspecto y funcionalidad similar a las listas [].

-- ejercicio 8)
data ListaAsoc a b = Vacia | Nodo a b (ListaAsoc a b)

-- a)
type Telefono = ListaAsoc String Int

-- b.1)
la_long :: ListaAsoc a b -> Int
la_long Vacia = 0
la_long (Nodo x y Vacia) = 1
la_long (Nodo a b (Nodo x y z)) = 1 + la_long(Nodo x y z)

-- b.2)
la_concat :: ListaAsoc a b -> ListaAsoc a b -> ListaAsoc a b
la_concat x Vacia = x
la_concat (Nodo x y z) a = Nodo x y (la_concat z a)

-- b.3)
la_agregar :: ListaAsoc a b -> a -> b -> ListaAsoc a b
la_agregar x y z = Nodo y z x 

-- b.4)
la_pares :: ListaAsoc a b -> [(a, b)]
la_pares Vacia = []
la_pares (Nodo a b c) = (a,b):(la_pares c)

-- b.5)
la_busca :: Eq a => ListaAsoc a b -> a -> Maybe b
la_busca Vacia p = Nothing
la_busca (Nodo x y z) p | x == p = Just y
                        | otherwise = la_busca z p

-- b.6)
la_borrar :: Eq a => a -> ListaAsoc a b -> ListaAsoc a b
la_borrar p Vacia = Vacia
la_borrar p (Nodo x y z) | x == p = la_borrar p z
                         | otherwise = Nodo x y (la_borrar p z)

-- Ejercicio 9)
data Arbol a = Hoja | Rama (Arbol a) a (Arbol a)
type Prefijos = Arbol String

-- a)
a_long :: Arbol a -> Int
a_long Hoja = 0
a_long (Rama x y z) = 1 + (a_long x) + (a_long z)

-- b)
a_hojas :: Arbol a -> Int
a_hojas Hoja = 1
a_hojas (Rama x y z) = (a_hojas x) + (a_hojas z)

-- c)
a_inc :: Num a => Arbol a -> Arbol a
a_inc Hoja = Hoja
a_inc (Rama x y z) = Rama (a_inc x) (y+1) (a_inc z)

-- d)
a_map :: (a -> b) -> Arbol a -> Arbol b
a_map f Hoja = Hoja
a_map f (Rama x y z) = Rama (a_map f x) (f y) (a_map f z)

a_incmap ar = a_map (+1) ar