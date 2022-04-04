#pragma once

class CLensFlareEnt : public CPointEntity
{
public:
	virtual void Spawn(void);
	virtual void KeyValue(KeyValueData*);

	void sendMessage();
};
