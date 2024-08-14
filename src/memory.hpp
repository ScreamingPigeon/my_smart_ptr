
template <typename T> class smart_ptr {
    public:
        T *data;
        // Constructor
        smart_ptr();

        // Destructor
        ~smart_ptr();

        // Operator Overloading
        bool operator=(smart_ptr other_ptr);

        // Modifiers
        void reset();
        void swap();
};

template <typename T> class unique_ptr : smart_ptr<T> {
        unique_ptr release();
        T get();
        void *get_deleter(); // Strange
        explicit operator bool();
        T *operator*();
        T *operator->();
        T *operator[](int i); // Strange
};

template <typename T> class shared_ptr : smart_ptr<T> {
        T get();
        explicit operator bool();
        T *operator*();
        T *operator->();
        T *operator[](int i); // Strange
        T owner_before();
        T owner_hash();
        T owner_equal();
        int use_count();
};

template <typename T> class weak_ptr : smart_ptr<T> {
        T get();
        T owner_before();
        T owner_hash();
        T owner_equal();
        int use_count();
        int expired();
        int lock();
};
