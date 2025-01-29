function formatStats(statsString) {
	try {

		// First, parse the string as a JSON
		const statsObject = JSON.parse(statsString);

		// Then convert to a more readable format
		return Object.entries(statsObject)
			.map(([key, value]) => ({ key, value }) )
	} catch (error) {
		console.error('Error parsing stats:', error);
		return 'Unable to parse stats';
	}
}

const handleError = (error) => {
	console.error('Error:', error);
	document.getElementById('response').innerText = 'Error occurred: Please try again';
}
