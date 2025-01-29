require 'webrick'
require 'socket'
require 'json'

# Defining server port
port = 8080

# Function to communicate with the C++ backend.
def communicate_with_cpp(command)
	cpp_host = 'localhost'
	cpp_port = 9000

	begin
		socket = TCPSocket.open(cpp_host, cpp_port)
		socket.puts(command) # Send command
		response = socket.gets # Read response
		socket.close
		response.strip
	rescue => e
		"Error communicating with C++ backend: #{e.message}"
	end

end

# Creating a new WEBrick HTTP server
server = WEBrick::HTTPServer.new(Port: port, DocumentRoot: File.join(Dir.pwd, 'public'))

# Mount handler for the main page
server.mount_proc '/' do |req, res|
	# Read the main html file
	content = File.read(File.join(Dir.pwd, 'views', 'index.html'))
	res.body = content
	res['Content-Type'] = 'text/html'
end

server.mount('/public', WEBrick::HTTPServlet::FileHandler, 'public')

# Handle POST requests from the webpage
server.mount_proc '/command' do |req, res|
	if req.request_method == 'POST'
		payload = JSON.parse(req.body) rescue {}
		command = payload['command']
		puts "payload['command']:#{command}"

		# Communicate with the C++ backend
		response_from_cpp = communicate_with_cpp(command)
		end_of_command_index = response_from_cpp.index("{")
		command_response = response_from_cpp[0..end_of_command_index-1]

		end_of_tower_stats_index = response_from_cpp.index("}", end_of_command_index)
		tower_stats_response = response_from_cpp[end_of_command_index..end_of_tower_stats_index]

		end_of_enemy_stats_index = response_from_cpp.index("}", end_of_tower_stats_index+1)
		enemy_stats_response = response_from_cpp[end_of_tower_stats_index+1..end_of_enemy_stats_index]

		enemy_name_response = response_from_cpp[end_of_enemy_stats_index+1..]

		res.body = { message: command_response, 
					 towerStats: tower_stats_response, 
					 enemyName: enemy_name_response,
					 enemyStats: enemy_stats_response					 
		}.to_json
		puts "res.body:#{res.body}"
		res['Content-Type'] = 'application/json'
	else
		res.body = { message: 'Invalid request method' }.to_json
		res['Content-Type'] = 'application/json'
	end
end
	

# Trap interrupt signal to shut down the server gracefully
trap 'INT' do
	server.shutdown
end

puts "Server is running at http://localhost:#{port}"
server.start


