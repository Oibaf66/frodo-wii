#ifndef __TIMER_HH__
#define __TIMER_HH__

class TimeoutHandler;

class TimerController
{
public:
	TimerController();

	int arm(TimeoutHandler *which, int ms);

	void disarm(TimeoutHandler *which);

	void tick();

	/* Singleton */
	static TimerController *controller;
	static void init();

private:
	int n_handlers;
	TimeoutHandler **handlers;
};

class TimeoutHandler
{
	friend class TimerController;
public:
	TimeoutHandler()
	{
		this->timeout = 0;
		this->timer_id = -1;
	}

	~TimeoutHandler();

	void tick()
	{
		this->timeout--;
		if (this->timeout == 0)
			this->timeoutCallback();
	}

	virtual void timeoutCallback() = 0;

protected:
	int timeout;
	int timer_id;
};

#endif /* __TIMER_HH__ */
