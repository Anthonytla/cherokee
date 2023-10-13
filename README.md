# Groupe de la_t 1003935

if needed, install readline library
`sudo apt-get install libreadline-dev`

to clean
`make fclean`

to clean and compile
`make re`

to start project
`./cherokee 3490`

connect in browser to 
http://localhost:3490/files/folder/test.html

# bench

siege -c 255 -t 10s "127.0.0.1:3490/files/index.html"
valgrind --leak-check=full ./cherokee 3490

# dynamic library

```
cd ./dynamic
make all

make clean
```

## Pour référence :
```
gcc -c -fPIC dynamic.c -o dynamic.o
gcc -shared dynamic.o -o libdynamic.so
```

Pour faire appel à une méthode depuis une librairie dynamique, placer le .so dans le dossier ./dynamic/ puis envoyer une requête CUSTOM avec le pattern :
> http://localhost:3490/<'lib.so'>/<'method'>
et le paramètre dans le body de la requête

ex:
http://localhost:3490/libdynamic.so/first_char

## Tests unitaires

Compiler avec `make test`
Executer avec `./test2`

## Tests fonctionnels

Compiler avec `make e2e`
Executer avec `./test2`