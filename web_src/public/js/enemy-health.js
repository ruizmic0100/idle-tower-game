document.addEventListener('DOMContentLoaded', () => {

	const enemyHealthBar = document.getElementById('enemy-health-bar');
	const enemyHealthText = document.getElementById('enemy-health-text');

	// Updates enemy health
	function updateEnemyHealth(currentHealth, maxHealth = 100) {
		const healthPercentage = (currentHealth / maxHealth) * 100;

		enemyHealthBar.style.width = `${healthPercentage}%`;

		enemyHealthText.textContent = `${currentHealth} HP`;

		// Color grading based on health percentage.
		if (healthPercentage > 66) {
			enemyHealthBar.style.backgroundColor = 'green';
		} else if (healthPercentage > 33) {
			enemyHealthBar.style.backgroundColor = 'orange';
		} else {
			enemyHealthBar.style.backgroundColor = 'red';
		}
	}

	updateEnemyHealth(100);

	setTimeout(() => updateEnemyHealth(50), 2000);
	setTimeout(() => updateEnemyHealth(10), 4000);

});
