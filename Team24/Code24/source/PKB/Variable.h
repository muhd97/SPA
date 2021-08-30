


class Variable {
public:
	using SharedPtr = std::shared_ptr<Variable>;

	SharedPtr create(string name) {
		return SharedPtr(new Variable(name));
	}

	string mName;

protected:
	Variable(string name) {
		mName = name;
	}
};