#pragma once

class Command
{
public:
	virtual ~Command() { }

	virtual bool Execute() = 0;
	virtual bool Undo() = 0;
};