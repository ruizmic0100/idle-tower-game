#!/bin/bash

# Function to display usage instructions.
usage() {
	printf "Usage: %s <cpp_file>\n" "$(basename "$0")" >&2
	return 1
}

# Function to compile the C++ file.
compile_cpp() {
	local cpp_file="$1"
	local output_file="a.out"

	# Validate file existence
	if [[ ! -f "$cpp_file" ]]; then
		printf "Error: File '%s' does not exist.\n" "$cpp_file" >&2
		return 1
	fi

	# Compile the file
	if ! g++ -o "$output_file" "$cpp_file"; then
		printf "Error: Failed to compile '%s'.\n" "$cpp_file" >&2
		return 1
	fi

	printf "Compilation successful. Executable: %s\n" "$output_file"
	return 0
}

# Function to run the compiled executable
run_executable() {
	local executable="$1"

	if [[ ! -x "$executable" ]]; then
		printf "Error: Executable '%s' not found or not executable.\n" "$executable" >&2
		return 1
	fi

	printf "Running the executable...\n"
	"./$executable"
	return $?
}

main() {
	# Validate input arguments
	if [[ $# -ne 1 ]]; then
		usage
		return 1
	fi

	local cpp_file="$1"
	local executable="a.out"

	# Comile and run the C++ file
	if ! compile_cpp "$cpp_file"; then
		return 1
	fi

	if ! run_executable "$executable"; then
		return 1
	fi
}

# Entry point
main "$@"
