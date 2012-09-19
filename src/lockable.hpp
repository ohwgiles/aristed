#ifndef LOCKABLE_HPP
#define LOCKABLE_HPP

#include <QMutex>

template<typename T>
class Lockable {
private:
	class ScopedLock {
	public:
		ScopedLock(QMutex& mutex) :mutex(mutex) { mutex.lock(); }
		~ScopedLock() { mutex.unlock(); }
	private:
		QMutex& mutex;
	};
public:
	ScopedLock scopedLock() { return ScopedLock(m); }
	T& operator()() { return v; }
	T& operator*() { return v; }
	T* operator->() { return &v; }
	void lock() { m.lock(); }
	void unlock() { m.unlock(); }
private:
	T v;
	QMutex m;
};

#endif // LOCKABLE_HPP
