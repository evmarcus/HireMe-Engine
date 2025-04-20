#include "TemplateDB.h"

Actor* TemplateDB::GetTemplate(std::string templateName) {
	// If the template already exists, retrieve it and return
	if (templates.find(templateName) != templates.end()) {
		return templates[templateName];
	}

	// Get the path of the current executable
	fs::path exePath = fs::current_path();

	fs::path templateDir = exePath / "resources" / "actor_templates" / (templateName + ".template");

	if (!fs::exists(templateDir)) {
		std::cout << "error: template " << templateName << " is missing";
		exit(0);
	}

	//int actorID = GameData::GetUUID();

	rapidjson::Document templateDoc;
	EngineUtils::ReadJsonFile(templateDir.string(), templateDoc);

	templates[templateName] = new Actor(templateDoc, -1);

	return templates[templateName];
}