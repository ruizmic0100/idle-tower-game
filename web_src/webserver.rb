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
server = WEBrick::HTTPServer.new(Port: port, DocumentRoot: './public')

# Create routes
server.mount_proc '/' do |req, res|
	res.body = <<-HTML
		<!DOCTYPE html>
		<html>
		<head>
			<title>Idle Tower Defense</title>
		</head>
		<body>
			<h1>Idle Tower Defense</h1>
			<button onclick="sendCommand('upgrade attackspeed')">Upgrade AttackSpeed</button>
			<button onclick="sendCommand('upgrade power')">Upgrade Power</button>
			<button onclick="sendCommand('upgrade gold')">Upgrade Gold</button>
			<div id="response"></div>
			<script>
				function fetchandDisplayData() {
					fetch('/command', {
						method: 'POST',
						headers: { 'Content-Type': 'application/json' },
						body: JSON.stringify({ command: 'getData' })	
					})
						.then(response => response.json())
						.then(data => {
							document.getElementById('towerstats').innerText = 'Tower Stats: ' + data.towerStats;
							document.getElementById('enemyname').innerText = data.enemyName;
							document.getElementById('enemystats').innerText = 'Enemy Stats: ' + data.enemyStats;
						})
						.catch(error => console.error('Error:', error));
				}


				// Fetch and Display data every 1 second
				setInterval(fetchandDisplayData, 60);

			</script>
			<script>
				function sendCommand(command) {
					fetch('/command', {
						method: 'POST',
						headers: { 'Content-Type': 'application/json' },
						body: JSON.stringify({ command: command })
					})
					.then(response => response.json())
					.then(data => {
						document.getElementById('response').innerText = 'Response: ' + data.message;
					})
					.catch(error => console.error('Error:', error));
				}

			
			</script>
			<div id="towerstats"></div>
			<div id="enemyname"></div>
			<div id="enemystats"></div>
		</body>
		</html>
	HTML
	res['Content-Type'] = 'text/html'
end

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


