#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include "Files.h"

class Score {
protected:
    int score_;
public:
    Score() : score_(0) {}
    virtual ~Score() = default;

    virtual void reset() { score_ = 0; }
    virtual void add(int points) { score_ += points; }
    virtual void set(int value) { score_ = value; }
    virtual int get() const { return score_; }
    virtual bool operator>(int rhs) const { return score_ > rhs; }
};

class HighScore : public Score {
private:
    corezone::GameDataManager& gameData_;
public:
    HighScore(corezone::GameDataManager& gameData);
    void load();
    void save();
    bool isNewHighScore(int currentScore) const;

    void set(int value) override;
};

#endif
