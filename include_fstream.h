class ifstream {
	public:
		void close();
		bool eof();
		void ignore();
		void open(char *);
};
class ofstream {
	public:
		void close();
		void open(char *);
};
