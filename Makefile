# Descargue el nSystem desde:
# https://users.dcc.uchile.cl/~lmateu/CC4302/download/nsystem64-beta2.tgz
# Ud. debe descomprimir este archivo con:
# $ tar zxvf nsystem64-beta2.tgz
# Compile el nSystem con:
# $ cd nsystem64-beta2/src
# $ make

# Importante:
# Para compilar exitosamente su tarea necesita definir la variable de
# ambiente NSYSTEM con:
# $ export NSYSTEM=.../nsystem64-beta2
# (debe reemplazar ... por la ruta para llegar a nsystem64-beta2)

# Programe su tarea en el archivo subasta.c y luego pruebela con:
# $ cd <el directorio en donde se encuentra este archivo>
# $ make test-subasta
# $ ./test-subasta

# Programe su test de prueba personal en test2-subasta.c y pruebelo con:
# $ make test2-subasta
# $ ./test2-subasta

LIBNSYS= $(NSYSTEM)/lib/libnSys.a

# Con opciones de debugging
CFLAGS= -Wall -pedantic -std=c99 -g -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LFLAGS= -g

# Con opciones de optimizacion:
#CFLAGS= -O2 -I$(NSYSTEM)/include -I$(NSYSTEM)/src
#LFLAGS=

test-subasta: test-subasta.o subasta.o
	gcc -g test-subasta.o subasta.o -o test-subasta $(LIBNSYS)

test2-subasta: test2-subasta.o subasta.o
	gcc -g test2-subasta.o subasta.o -o test2-subasta $(LIBNSYS)

test-subasta.o test2-subasta.o subasta.o: subasta.h

clean:
	rm -f *.o test-subasta tes2t-subasta
