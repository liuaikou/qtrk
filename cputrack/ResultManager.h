
#pragma once

#include "QueuedTracker.h"
#include <list>
#include "threads.h"


class ResultFile
{
public:
	ResultFile() { }
	virtual ~ResultFile() {}
	virtual void LoadRow(std::vector<vector3f>& pos) = 0;
	virtual void SaveRow(std::vector<vector3f>& pos) = 0;
};

class TextResultFile : public ResultFile
{
public:
	TextResultFile(const char* fn, bool write);
	void LoadRow(std::vector<vector3f>& pos);
	void SaveRow(std::vector<vector3f>& pos);
private:
	FILE *f;
};

class BinaryResultFile : public ResultFile
{
public:
	BinaryResultFile(const char* fn, bool write);
	void LoadRow(std::vector<vector3f>& pos);
	void SaveRow(std::vector<vector3f>& pos);
protected:
	FILE *f;
};


// Labview interface packing
#pragma pack(push,1)
struct ResultManagerConfig
{
	int numBeads, numFrameInfoColumns;
	vector3f scaling;
	vector3f offset; // output will be (position + offset) * scaling
	int writeInterval; // [frames]
	uint maxFramesInMemory; // 0 for infinite
	uint8_t binaryOutput;
};
#pragma pack(pop)

// Runs a seperate result fetching thread
class ResultManager
{
public:
	ResultManager(const char *outfile, const char *frameinfo, ResultManagerConfig *cfg, std::vector<std::string> colnames);
	~ResultManager();

	void SaveSection(int start, int end, const char *beadposfile, const char *infofile);
	void SetTracker(QueuedTracker *qtrk);
	QueuedTracker* GetTracker();

	int GetBeadPositions(int startFrame, int endFrame, int bead, LocalizationResult* r);
	int GetResults(LocalizationResult* results, int startFrame, int numResults);
	void Flush();

	struct FrameCounters {
		FrameCounters();
		int startFrame; // startFrame for frameResults
		int processedFrames; // frame where all data is retrieved (all beads)
		int lastSaveFrame;
		int capturedFrames;  // lock by resultMutex
		int localizationsDone;
		int lostFrames;
		int fileError;
	};

	FrameCounters GetFrameCounters();
	void StoreFrameInfo(int frame, double timestamp, float* columns); // return #frames
	int GetFrameCount();
	// Make sure that the space for that frame is allocated

	bool RemoveBeadResults(int bead);
	
	const ResultManagerConfig& Config() { return config; }

protected:
	bool CheckResultSpace(int fr);
	void Write();
	void WriteBinaryResults();
	void WriteTextResults();

	void StoreResult(LocalizationResult* r);
	static void ThreadLoop(void *param);
	bool Update();
	void WriteBinaryFileHeader();

	struct FrameResult
	{
		FrameResult(int nResult, int nFrameInfo) : frameInfo(nFrameInfo), results(nResult) { count=0; timestamp=0; hasFrameInfo=false;}
		std::vector<LocalizationResult> results;
		std::vector<float> frameInfo;
		int count;
		double timestamp;
		bool hasFrameInfo;
	};

	Threads::Mutex resultMutex, trackerMutex;

	std::vector<std::string> frameInfoNames;

	std::deque< FrameResult* > frameResults;
	FrameCounters cnt;
	ResultManagerConfig config;

	ResultFile* resultFile;

	QueuedTracker* qtrk;

	std::string outputFile, frameInfoFile;
	Threads::Handle* thread;
	Atomic<bool> quit;

};
