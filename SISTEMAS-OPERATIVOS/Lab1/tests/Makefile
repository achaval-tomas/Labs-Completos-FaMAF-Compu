# La forma normal de usar este Makefile debería ser correr
# "make test" EN EL DIRECTORIO DE ARRIBA, no en este.
CPPFLAGS+= -I..
LDFLAGS+= `pkg-config --libs check`

TARGETS=runner runner-command runner-parsing leaktest
SOURCES=$(shell echo *.c)

# Modulos que ya se compilaron
COMMON_OBJECTS=../command.o ../strextra.o

ARCHDIR=objects-$(shell uname -m)

vpath parser.o ../$(ARCHDIR)
vpath lexer.o ../$(ARCHDIR)
PARSER_OBJECTS=parser.o lexer.o ../parsing.o

# Al modulo ejecutor lo recompilamos en este directorio usando mocks
MOCK_OBJECTS=builtin.o execute.o syscall_mock.o
vpath execute.c ..
vpath builtin.c ..
execute.o: CPPFLAGS += -DREPLACE_SYSCALLS=1
builtin.o: CPPFLAGS += -DREPLACE_SYSCALLS=1


# - Cada test suite linkea lo minimo posible
# - Los runners usan la implementacion de referencia
#   de los modulos que no estan bajo prueba
runner: run_tests.o test_scommand.o test_pipeline.o test_execute.o test_parsing.o $(COMMON_OBJECTS) $(PARSER_OBJECTS) $(MOCK_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

runner-command: run_command.o test_scommand.o test_pipeline.o $(COMMON_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

runner-parsing: run_parsing.o test_parsing.o $(COMMON_OBJECTS) $(PARSER_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

leaktest: leaktest.o test_scommand.o test_pipeline.o $(COMMON_OBJECTS) $(PARSER_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)


# Cada runner usa partes distintas de run_tests.c
run_tests.o:   CPPFLAGS+= -DTEST_COMMAND -DTEST_EXECUTE -DTEST_PARSER

run_command.o: CPPFLAGS+= -DTEST_COMMAND
run_command.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<


run_parsing.o: CPPFLAGS+= -DTEST_PARSER
run_parsing.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

# Ejecutar runners
test: runner
	./$^

test-command: runner-command
	./$^

test-parsing: runner-parsing
	./$^

memtest: leaktest
	# Ver http://live.gnome.org/Valgrind para explicacion de G_SLICE y G_DEBUG
	# Uso el archivo glib.supp para no ver los errores internos de GLib2
	G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --leak-check=full --show-reachable=yes --suppressions=glib.supp ./$^

.PHONY: all clean test test-command memtest

all: test

clean:
	rm -f $(TARGETS) *.o test*.log .depend *~

.depend: $(SOURCES)
	$(CC) $(CPPFLAGS) -MM $^ > $@

-include .depend
