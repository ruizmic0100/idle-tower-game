// Main server-side game logic and event handlers.
class IdleTowerDefense {
	constructor() {
		this.initializeEventListeners();
		this.startGameLoop();
	}

	initializeEventListeners() {
		// Adding event listeners to buttons
		document.getElementById('upgrade-attackspeed').addEventListener('click', () =>
			this.sendCommand('upgrade attackspeed'));
		document.getElementById('upgrade-power').addEventListener('click', () =>
			this.sendCommand('upgrade power'));
		document.getElementById('upgrade-gold').addEventListener('click', () =>
			this.sendCommand('upgrade gold'));
	}

	async sendCommand(command) {
		try {
			const response = await fetch('/command', {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({ command: command })
			});
			const data = await reponse.json();
			document.getElementById('response').innerText = 'Response: ' + data.message;
		} catch (error) {
			handleError(error);
		}
	}


	async fetchAndDisplayData() {
		try {
			const response = await fetch('/command', {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({ command: 'getData' })
			});
			const data = await response.json();
			
			const towerStats = formatStats(data.towerStats);
			
			// Update display elements
			const power_pair = towerStats.find(e => e.key === 'power');
			const attackspeed_pair = towerStats.find(e => e.key === 'attackspeed');
			const gold_pair = towerStats.find(e => e.key === 'gold');
			const goldpersecond_pair = towerStats.find(e => e.key === 'goldpersecond');
			const enemies_defeated_pair = towerStats.find(e => e.key === 'enemiesdefeated');


			document.getElementById('player-gold-val').innerText = gold_pair.value;
			document.getElementById('tower-goldpersecond-val').innerText = goldpersecond_pair.value;
			document.getElementById('tower-power-val').innerText = power_pair.value;
			document.getElementById('tower-attackspeed-val').innerText = attackspeed_pair.value;
			document.getElementById('enemies-defeated-val').innerText = enemies_defeated_pair.value;
		} catch (error) {
			handleError(error);
		}

	}

	startGameLoop() {
		// Update every 60ms
		setInterval(() => this.fetchAndDisplayData(), 250);
	}
}

// Initialize the game when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
	new IdleTowerDefense();
});
