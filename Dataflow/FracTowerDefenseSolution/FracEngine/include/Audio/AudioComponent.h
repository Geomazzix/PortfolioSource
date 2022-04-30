#pragma once

namespace FMOD {
	namespace Studio {
		class EventInstance;
		class EventDescription;
	}
}

namespace Frac
{
	struct AudioSourceComponent
	{
		std::unordered_map<std::string, FMOD::Studio::EventInstance*> events;
		std::unordered_map<std::string, FMOD::Studio::EventDescription*> descriptions;
	};

	/*struct AudioListenerComponent
	{
		FMOD_3D_ATTRIBUTES attributes;
	};*/
} // namespace Frac
