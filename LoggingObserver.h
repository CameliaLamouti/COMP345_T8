// LoggingObserver.h
// COMP 345 - Assignment 2 - Part 5: Game log observer (commands and orders)
//
// Observer design pattern used to write a game log:
//   - ILoggable : interface of every object that can be logged (stringToLog()).
//   - Subject   : object that keeps a list of observers and notifies them.
//   - Observer  : object that gets notified by a subject (Update()).
//   - LogObserver : concrete observer that writes stringToLog() of the
//                   notifying object into the log file "gamelog.txt".
//
// Command, CommandProcessor, Order, OrdersList and GameEngine all inherit
// from both Subject and ILoggable, and call Notify(this) in saveEffect(),
// saveCommand(), execute(), addOrder() and transition() respectively.
//
// Ownership: a Subject does NOT own its observers. The observers must stay
// alive as long as the subjects they are attached to (or be detached first).

#ifndef LOGGINGOBSERVER_H
#define LOGGINGOBSERVER_H

#include <list>
#include <ostream>
#include <string>

// ============================================================
// ILoggable : interface of every class whose objects can be logged.
// ============================================================
class ILoggable {
public:
    ILoggable();                                   // Default constructor
    ILoggable(const ILoggable& other);             // Copy constructor
    ILoggable& operator=(const ILoggable& other);  // Assignment operator
    virtual ~ILoggable();                          // Destructor

    // Creates and returns the line to write in the log file.
    virtual std::string stringToLog() const = 0;

    friend std::ostream& operator<<(std::ostream& out, const ILoggable& loggable); // Stream insertion operator
};

// ============================================================
// Observer : abstract observer, notified by the subjects it is attached to.
// ============================================================
class Observer {
public:
    Observer();                                  // Default constructor
    Observer(const Observer& other);             // Copy constructor
    Observer& operator=(const Observer& other);  // Assignment operator
    virtual ~Observer();                         // Destructor

    // Called by Subject::Notify() with the object that changed.
    virtual void Update(ILoggable* loggable) = 0;

    friend std::ostream& operator<<(std::ostream& out, const Observer& observer); // Stream insertion operator
};

// ============================================================
// Subject : keeps a list of observers and notifies them of changes.
// ============================================================
class Subject {
public:
    Subject();                                 // Default constructor: no observers
    Subject(const Subject& other);             // Copy constructor: the copy is watched by the same observers
    Subject& operator=(const Subject& other);  // Assignment operator: same observers as the other subject
    virtual ~Subject();                        // Destructor: frees the list, not the observers

    void Attach(Observer* observer);           // Adds an observer (ignores nullptr and duplicates)
    void Detach(Observer* observer);           // Removes an observer
    void Notify(ILoggable* loggable);          // Calls Update(loggable) on every attached observer

    void attachObserversTo(Subject* other) const;  // Attaches every observer of this subject to another subject
    int  getObserverCount() const;                 // Returns the number of attached observers

    friend std::ostream& operator<<(std::ostream& out, const Subject& subject); // Stream insertion operator

private:
    std::list<Observer*>* observers;   // owned list of NOT owned observers
};

// ============================================================
// LogObserver : writes every notification into the game log file.
// ============================================================
class LogObserver : public Observer {
public:
    static const std::string DEFAULT_FILE_NAME;      // "gamelog.txt"

    LogObserver();                                   // Starts a new, empty "gamelog.txt"
    explicit LogObserver(const std::string& fileName);  // Starts a new, empty log file with the given name
    LogObserver(const LogObserver& other);           // Copy constructor: writes to the same file
    LogObserver& operator=(const LogObserver& other);  // Assignment operator
    ~LogObserver();                                  // Destructor

    // Appends loggable->stringToLog() as a new line of the log file.
    void Update(ILoggable* loggable) override;

    std::string getFileName() const;                 // Returns the name of the log file

    friend std::ostream& operator<<(std::ostream& out, const LogObserver& observer); // Stream insertion operator

private:
    std::string* fileName;   // owned
};

#endif // LOGGINGOBSERVER_H
