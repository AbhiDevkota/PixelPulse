#include <cricket/ScoreBoard.h>
#include <iostream>

ScoreBoard::ScoreBoard(const std::string& fontPath){	//To use the regular.tff font
	if (!font.openFromFile(fontPath)) {
		std::cout << "Failed to load font from " << fontPath << std::endl;
	}

	//RUNS TEXT
	runsText.setFont(font);
	runsText.setCharacterSize(32);		//The Font Size
	runsText.setFillColor(sf::Color::White);
	runsText.setPosition({30.f, 30.f});

	//WICKETS TEXT
	wicketsText.setFont(font);
	wicketsText.setCharacterSize(32);		//The Font Size
	wicketsText.setFillColor(sf::Color::White);
	wicketsText.setPosition({ 30.f, 75.f });


	//LAST SHOT TEXT
	lastShotText.setFont(font);
	lastShotText.setCharacterSize(52);		//The Font Size
	lastShotText.setFillColor(sf::Color(255, 220, 50)); //custom color i.e gold kinda (RCB number color)
	lastShotText.setPosition({350.f,250.f});

	updateDisplayText();
}

void ScoreBoard::addRun(float timingScore) {
	int scored = calculateRuns(timingScore);
	runs += scored;

	if (scored == 6) {
		lastShotText.setString("SIX!");

	}
	else if (scored == 4) {
		lastShotText.setString("FOUR!");
	}
	else {
		lastShotText.setString(std::to_string(scored) + " RUNS");

	}

	lastShotTimer = 1.2f; //Can we adjust. For now it show for 1.2 sec

	updateDisplayText();
}

void ScoreBoard::addWicket(){
	wickets++;
	lastShotText.setString("OUT!");
	lastShotText.setFillColor(sf::Color::Red);
	lastShotTimer = 1.5f; //Can we adjust. For now it show for 1.5 sec
	updateDisplayText();
}


int ScoreBoard::getRuns() const{
	return runs;
}

int ScoreBoard::getWickets() const{
	return wickets;
}

bool ScoreBoard::isGameOver() const{
	return wickets >= maxWickets;
}


void ScoreBoard::draw(sf::RenderWindow& window){
	window.draw(runsText);
	window.draw(wicketsText);

	if (lastShotTimer > 0.f){	//show last shot while timer is active
		window.draw(lastShotText);
	}

}



void ScoreBoard::update(float dt){
	if (lastShotTimer > 0.f){

		lastShotTimer -= dt;

		float alpha = (lastShotTimer / 1.2f) * 255.f;
		if (alpha < 0.f) alpha = 0.f;
		
		sf::Color c = lastShotText.getFillColor();
		c.a = static_cast<uint8_t>(alpha);
		lastShotText.setFillColor(c);

	}
}


int ScoreBoard::calculateRuns(float timingScore) const{
	if (timingScore >= 0.85f) return 6;
	else if (timingScore >= 0.60f) return 4;
	else if (timingScore >= 0.30f) return 2;
	else return 1;


	/*For the calculation, it change the delta time into the score
		like this
		0.0 sec to 0.3 sec = 1 run
		0.3 sec to 0.6 sec = 2 runs
		0.6 to 0.85 = 4 runs
		0.85 to 1.0 = 6 runs
	*/
		
}


void ScoreBoard::updateDisplayText(){
	runsText.setString("RUNS: " + std::to_string(runs));
	wicketsText.setString(
		"WICKETS: " + std::to_string(wickets)
	+
	"/" + std::to_string(maxWickets));
}