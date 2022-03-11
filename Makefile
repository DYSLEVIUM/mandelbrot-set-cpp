cpp_version = c++2a
src_file = ./src/main.cc
executable = ./src/main.exe
deps = ./deps/SFML/SFML-2.5.1

run: $(executable)
	./$(executable)

$(executable): $(src_file) $(deps)
	g++ -std=$(cpp_version) $(src_file) -L $(deps) -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -o $(executable)

clean:
	rm -rf $(executable)