#ifndef LOCKABLE_HPP
#define LOCKABLE_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
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
