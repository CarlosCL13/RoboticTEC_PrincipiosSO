CC=gcc
CFLAGS=-Wall

# Rutas a los archivos fuente
SERVIDOR_SRC=Servidor/servidor.c
CLIENTE_SRC=Cliente/cliente.c
NODO1_SRC=Nodos/nodo1.c
NODO2_SRC=Nodos/nodo2.c
NODO3_SRC=Nodos/nodo3.c

# Ruta a la biblioteca Arduino
ARDUINO_LIB=Arduino/Biblioteca/arduino_lib.a

# Ejecutables (con ruta de salida)
SERVIDOR=Servidor/servidor
CLIENTE=Cliente/cliente
NODO1=Nodos/nodo1
NODO2=Nodos/nodo2
NODO3=Nodos/nodo3

all: $(SERVIDOR) $(CLIENTE) $(NODO1) $(NODO2) $(NODO3)

$(SERVIDOR): $(SERVIDOR_SRC)
	$(CC) $(CFLAGS) -o $@ $< -lpthread $(ARDUINO_LIB)

$(CLIENTE): $(CLIENTE_SRC)
	$(CC) $(CFLAGS) -o $@ $<

$(NODO1): $(NODO1_SRC)
	$(CC) $(CFLAGS) -o $@ $<

$(NODO2): $(NODO2_SRC)
	$(CC) $(CFLAGS) -o $@ $<

$(NODO3): $(NODO3_SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(SERVIDOR) $(CLIENTE) $(NODO1) $(NODO2) $(NODO3)