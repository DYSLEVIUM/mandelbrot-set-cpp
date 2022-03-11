cpp_version = c++2a
src_file = ./src/main.cc
executable = ./src/main.exe
deps = deps/SFML/SFML-2.5.1

run: $(executable)
	$(executable) > output.log 2> error.log

$(executable): $(src_file) $(deps)
	g++ -std=$(cpp_version) -H $(src_file) -I $(deps)/include -L $(deps)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -o $(executable)

clean:
	rm -rf $(executable) *.log