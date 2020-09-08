#pragma

#include <vector>

namespace pge {
	template<class T>
	class SoftwareImage2D {
	private:
		std::vector<T> data;

		int width, height;

	public:
		void reset(int width, int height, T defaultValue = T());

		T getPixel(int x, int y) const {
			return data[x + y * width];
		}

		void setPixel(int x, int y, const T &value) {
			data[x + y * width] = value;
		}

		T* getData() {
			return &data[0];
		}

		int getWidth() const {
			return width;
		}

		int getHeight() const {
			return height;
		}
	};

	template<class T>
	void SoftwareImage2D<T>::reset(int width, int height, T defaultValue) {
		this->width = width;
		this->height = height;

		data.clear();
		data.assign(width * height, defaultValue);
	}
}