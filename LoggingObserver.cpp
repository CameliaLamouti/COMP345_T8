// LoggingObserver.cpp
// COMP 345 - Assignment 2 - Part 5: Game log observer (commands and orders)
// Implementation of the ILoggable, Observer, Subject and LogObserver classes
// declared in LoggingObserver.h.

#include "LoggingObserver.h"

#include <algorithm>
#include <fstream>
#include <iostream>

// -------------------------------- ILoggable -------------------------------- //

/**
 * Default constructor: an interface has no data to initialize.
 */
ILoggable::ILoggable() {}

/**
 * Copy constructor: an interface has no data to copy.
 */
ILoggable::ILoggable(const ILoggable& other) {
    (void)other;
}

/**
 * Assignment operator: an interface has no data to copy.
 */
ILoggable& ILoggable::operator=(const ILoggable& other) {
    (void)other;
    return *this;
}

/**
 * Destructor: virtual so that derived objects are destroyed correctly.
 */
ILoggable::~ILoggable() {}

/**
 * Stream insertion operator: prints the line the object would write in the log.
 */
std::ostream& operator<<(std::ostream& out, const ILoggable& loggable) {
    out << loggable.stringToLog();
    return out;
}

// -------------------------------- Observer -------------------------------- //

/**
 * Default constructor: the base observer has no data to initialize.
 */
Observer::Observer() {}

/**
 * Copy constructor: the base observer has no data to copy.
 */
Observer::Observer(const Observer& other) {
    (void)other;
}

/**
 * Assignment operator: the base observer has no data to copy.
 */
Observer& Observer::operator=(const Observer& other) {
    (void)other;
    return *this;
}

/**
 * Destructor: virtual so that derived observers are destroyed correctly.
 */
Observer::~Observer() {}

/**
 * Stream insertion operator: prints a short description of the observer.
 */
std::ostream& operator<<(std::ostream& out, const Observer& observer) {
    (void)observer;
    out << "Observer";
    return out;
}

// --------------------------------- Subject --------------------------------- //

/**
 * Default constructor: creates a subject with no observers.
 */
Subject::Subject() : observers(new std::list<Observer*>()) {}

/**
 * Copy constructor: the copy gets its own list holding the same observers,
 * so a copied command or order is logged by the same log observer.
 */
Subject::Subject(const Subject& other)
    : observers(new std::list<Observer*>(*other.observers)) {}

/**
 * Assignment operator: replaces this subject's observers with the other
 * subject's observers (the observers themselves are shared, not copied).
 */
Subject& Subject::operator=(const Subject& other) {
    if (this != &other) {
        *observers = *other.observers;
    }
    return *this;
}

/**
 * Destructor: frees the list only. The observers are not owned by the subject.
 */
Subject::~Subject() {
    delete observers;
}

/**
 * Adds an observer to the list. Ignores nullptr and observers already attached,
 * so the same observer never writes the same entry twice.
 */
void Subject::Attach(Observer* observer) {
    if (observer == nullptr) {
        return;
    }
    if (std::find(observers->begin(), observers->end(), observer) == observers->end()) {
        observers->push_back(observer);
    }
}

/**
 * Removes an observer from the list (does nothing if it is not attached).
 */
void Subject::Detach(Observer* observer) {
    observers->remove(observer);
}

/**
 * Notifies every attached observer that the given loggable object changed.
 */
void Subject::Notify(ILoggable* loggable) {
    for (Observer* observer : *observers) {
        observer->Update(loggable);
    }
}

/**
 * Attaches every observer of this subject to another subject. Used so that the
 * commands made by a CommandProcessor and the orders put in an OrdersList are
 * logged by the same observers as the processor or the list.
 */
void Subject::attachObserversTo(Subject* other) const {
    if (other == nullptr || other == this) {
        return;
    }
    for (Observer* observer : *observers) {
        other->Attach(observer);
    }
}

/**
 * Returns the number of observers attached to this subject.
 */
int Subject::getObserverCount() const {
    return static_cast<int>(observers->size());
}

/**
 * Stream insertion operator: prints how many observers are attached.
 */
std::ostream& operator<<(std::ostream& out, const Subject& subject) {
    out << "Subject with " << subject.getObserverCount() << " observer(s)";
    return out;
}

// ------------------------------- LogObserver ------------------------------- //

const std::string LogObserver::DEFAULT_FILE_NAME = "gamelog.txt";

/**
 * Default constructor: starts a new, empty "gamelog.txt" for the current game.
 */
LogObserver::LogObserver() : LogObserver(DEFAULT_FILE_NAME) {}

/**
 * Creates a log observer writing to the given file. The file is emptied so the
 * log only holds the current game's entries.
 */
LogObserver::LogObserver(const std::string& fileName)
    : Observer(), fileName(new std::string(fileName)) {
    std::ofstream file(*this->fileName, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "LogObserver: could not open " << *this->fileName << std::endl;
    }
}

/**
 * Copy constructor: the copy writes to the same file. The file is not emptied.
 */
LogObserver::LogObserver(const LogObserver& other)
    : Observer(other), fileName(new std::string(*other.fileName)) {}

/**
 * Assignment operator: writes to the same file as the other observer from now on.
 */
LogObserver& LogObserver::operator=(const LogObserver& other) {
    if (this != &other) {
        Observer::operator=(other);
        *fileName = *other.fileName;
    }
    return *this;
}

/**
 * Destructor: frees the file name.
 */
LogObserver::~LogObserver() {
    delete fileName;
}

/**
 * Writes the loggable object's stringToLog() as a new line at the end of the
 * log file. The file is opened and closed on every entry, so each entry is
 * saved right away even if the program stops unexpectedly.
 */
void LogObserver::Update(ILoggable* loggable) {
    if (loggable == nullptr) {
        return;
    }
    std::ofstream file(*fileName, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "LogObserver: could not open " << *fileName << std::endl;
        return;
    }
    file << loggable->stringToLog() << std::endl;
}

/**
 * Returns the name of the log file.
 */
std::string LogObserver::getFileName() const {
    return *fileName;
}

/**
 * Stream insertion operator: prints the name of the log file.
 */
std::ostream& operator<<(std::ostream& out, const LogObserver& observer) {
    out << "LogObserver writing to " << observer.getFileName();
    return out;
}
