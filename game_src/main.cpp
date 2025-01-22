#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include "nlohmann/json.hpp" // Added a JSON library
#include <cctype>
#include <locale>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

using json = nlohmann::json;

// Networking Logic
#define PORT 9000

// Prototypes
class Enemy;
class GameHandler;
class Tower;

// trim from end (in place) helper function
inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}


// Non-blocking keyboard input setup for Unix-based systems.
#ifndef _WIN32
char kbhit() {
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();
	
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	return ch;

}
#endif



class Enemy {
private:


public:
	std::string name;
	int level;
	double health;
	double goldReward;

	Enemy(int level, int enemiesDefeated, int checkpoint) {
		std::vector<std::string> names = {"Goblin", "Orc", "Troll", "Dragon", "Demon"};
		name = names[level % names.size()];
		health = 10.0 * pow(1.2, level) + ((enemiesDefeated * 2) * std::max(0, enemiesDefeated - 2)) + (checkpoint * 200);
		goldReward = 5.0* pow(1.1, level);
	}

	std::unordered_map<std::string, double> getStats() {
		return {
			{"level", level},
			{"health", health},
			{"goldreward", goldReward}
		};
	}

};

class Tower {
private:

public:
	int level;
	double gold;
	double autoGoldPerSecond;
	double power;
	double attackSpeed;
	int enemiesDefeated;

	Tower() : level(1), power(10), gold(0), enemiesDefeated(0), autoGoldPerSecond(1), attackSpeed(1.0) {}

	bool upgradeAttackSpeed() {
		if (gold >= getUpgradeCost()) {
			level++;
			gold -= getUpgradeCost();
			attackSpeed = attackSpeed + 0.25;
			std::cout << "Upgraded Attack Speed!" << std::endl;
			return true;
		} else {
			std::cout << "Not enough gold." << std::endl;
			return false;
		}
	}

	bool upgradePower() {
		if (gold >= getUpgradeCost()) {
			level++;
			gold -= getUpgradeCost();
			power = power * 1.5;
			std::cout << "Upgraded Power!" << std::endl;
			return true;
		} else {
			std::cout << "Not enough gold." << std::endl;
			return false;
		}
	}

	bool upgradeGold() {
		if (gold >= getUpgradeCost()) {
			level++;
			gold -= getUpgradeCost();
			autoGoldPerSecond = autoGoldPerSecond * 1.2;
			std::cout << "Upgraded GoldPerSecond!" << std::endl;
			return true;
		} else {
			return false;
		}
	}

	double getUpgradeCost() const {
		return 10.0 * pow(1.5, level - 1);
	}

	void generateGold() {
		gold += autoGoldPerSecond;
	}

	void attack(Enemy* enemy) {
		std::cout << "Attacked!" << std::endl;
		enemy->health -= power;
	}

	bool battleResults(Enemy* enemy) {
		if (enemy->health <= 0.0) {
			gold += enemy->goldReward;
			enemiesDefeated++;
			return true;
		}
		return false;
	}

	void displayStats() const {

		std::cout << "\n=== IDLE TOWER GAME ===\n\n";
		std::cout << "Tower Level: " << level << std::endl;
		std::cout << "Tower Power: " << std::fixed << std::setprecision(1) << power << std::endl;
		std::cout << "Tower Attack Speed: " << std::fixed << std::setprecision(1) << attackSpeed << std::endl;
		std::cout << "Gold: " << std::fixed << std::setprecision(1) << gold << std::endl;
		std::cout << "Gold per second: " << std::fixed << std::setprecision(1) << autoGoldPerSecond << std::endl;
		std::cout << "Enemies Defeated: " << enemiesDefeated << std::endl;
		std::cout << "Upgrade Cost: " << std::fixed << std::setprecision(1) << getUpgradeCost() << std::endl;
		std::cout << "\nCommands:\n";
		std::cout << "u - Upgrade Tower\n";
		// std::cout << "f - Fight Enemy\n";
		std::cout << "q - Quit\n";
	}

	std::unordered_map<std::string, double> getStats() {
		return {
			{"power", power},
			{"attackspeed", attackSpeed},
			{"goldpersecond", autoGoldPerSecond},
			{"gold", gold},
			{"enemiesdefeated", enemiesDefeated},
			{"upgradecost", getUpgradeCost()}
		};
	}
};


class GameHandler {
private:
	bool running;

public:
	int checkpoint;
	Enemy* enemy;

	GameHandler() : checkpoint(0) {
		enemy = new Enemy(0, 0, 0);		
	}

	Enemy* createEnemy(int level, int enemiesDefeated) {
		delete enemy;
		enemy = new Enemy(level, enemiesDefeated, checkpoint);
		return enemy;
	}

	void displayEnemyStats() {
		std::cout << "\n\n=== ENEMY ===\n\n";
		std::cout << "Enemy Name: " << enemy->name << std::endl;
		std::cout << "Enemy Level: " << enemy->level << std::endl;
		std::cout << "Enemy Health: " << enemy->health << std::endl;
	}

	void displayCheckpointNumber() {
		std::cout << "\nCHECKPOINT: " << checkpoint << std::endl;
	}
		
};

// Handler for incoming commands from frontend.
void handleCommand(std::string& command, std::string& response, Tower& tower) {
	rtrim(command); // Have to right strip the command for some odd reason...
	std::cout << "command given: " << command << std::endl;
	
	if (command == "upgrade attackspeed") {
		if (tower.upgradeAttackSpeed()) {
        	response = "C++ Backend: upgrade attackspeed successful!";
		} else {
			response = "C++ Backend: upgrade attackspeed not successful.";
		}
    } else if (command == "upgrade power") {
		if (tower.upgradePower()) {
        	response = "C++ Backend: upgrade power successful!";
		} else {
			response = "C++ Backend: upgrade power not successful.";
		}
    } else if (command == "upgrade gold") {
		if (tower.upgradeGold()) {
        	response = "C++ Backend: upgrade gold successful!";
		} else {
			response = "C++ Backend: upgrade gold not successful.";
		}
	} else if (command == "getData") {
		response = "C++ Backend: getting Data!";
    } else {
        response = "C++ Backend: Unknown command.";
    }
}

int main() {
	// IPC logic inits.
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};

	// Game logic inits.
	Tower tower;
	GameHandler gmH;
	bool running = true;

	// Create socket file descriptor.
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}

	// Set socket options.
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Bind the socket
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	// Start listening for connections
	if (listen(server_fd, 3) < 0) {
		perror("Listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "C++ Backend listening on port " << PORT << "..." << std::endl;

	// Set up timer for while loop.
	auto lastUpdate = std::chrono::steady_clock::now();
	auto lastDisplay = std::chrono::steady_clock::now();
	auto lastAttack = std::chrono::steady_clock::now();

	
	while (running) {
		// Update timers.
		auto currentTime = std::chrono::steady_clock::now();
		auto timePassed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUpdate).count();
		auto displayTimePassed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastDisplay).count();
		auto attackTimePassed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastAttack).count();

		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
			perror("Accept");
			exit(EXIT_FAILURE);
		}

		// Read command from the socket.
		memset(buffer, 0, sizeof(buffer));
		read(new_socket, buffer, 1024);
		std::string command(buffer);
		std::string response;
		auto towerStatsData = tower.getStats();
		auto enemyStatsData = gmH.enemy->getStats();

		// Process the command.
		handleCommand(command, response, tower);
		
		// Process the stats into a json string.
		json j_umap(towerStatsData);
		std::string tower_j_umap = j_umap.dump();
		json j_enemy_umap(enemyStatsData);
		std::string enemy_j_umap = j_enemy_umap.dump();

		// Send the response back.
		send(new_socket, response.c_str(), response.length(), 0);
		send(new_socket, tower_j_umap.c_str(), tower_j_umap.length(), 0);
		send(new_socket, enemy_j_umap.c_str(), enemy_j_umap.length(), 0);
		send(new_socket, gmH.enemy->name.c_str(), gmH.enemy->name.length(), 0);
		close(new_socket);
		
		// (1) Do calculations first before displaying them for each update.
		if (tower.enemiesDefeated == (10 * (gmH.checkpoint + 1))) {
			gmH.checkpoint+=1;
			gmH.createEnemy(tower.level, tower.enemiesDefeated);
			std::cout << "\n\n=== New Checkpoint Reached! ===\n\n";
		}
		
		// 1 sec loop
		if (timePassed >= 1) {
			lastUpdate = currentTime;
			tower.generateGold();
		}

		// Auto attack loop:
		if (attackTimePassed >= (1000/tower.attackSpeed)) {
			tower.attack(gmH.enemy);
			lastAttack = currentTime;
		}
		
		// (2) Display stats.
		if (displayTimePassed >= 100) {
			lastDisplay = currentTime;
		}

		// (3) Battle Checking
		if (tower.battleResults(gmH.enemy)) {
			gmH.createEnemy(tower.level, tower.enemiesDefeated);
			std::cout << "New Enemy!" << std::endl;
		}


		// Check for input.
//		#ifdef _WIN32
//			if (_kbhit()) {
//				char comm = _getch();
//		#else
//			char comm = kbhit();
//			if (comm != EOF) {
//		#endif	
//				switch (command) {
//					case 'u':
//						tower.upgrade();
//						break;
//					case 'f':
//						if (tower.fightEnemy(gmH.enemy)) {
//							std::cout << " Victory!" << std::endl;
//						} else {
//							std::cout << " Enemy too strong!" << std::endl;
//						}
//						std::this_thread::sleep_for(std::chrono::milliseconds(500));
//						break;
//					case 'q':
//						running = false;
//						break;
				//}
			//}

		// Small delay to prevent CPU overuse.
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}



	return 0;
}








