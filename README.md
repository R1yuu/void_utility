# USAGE
These header files are meant to be a simple means of using datastructures in a C project.
They are universally useable with any other C datatype and are well integrated with each other.
All of the Header-Files Include DoxyGen Documentation which can and should be read before using them.

Values and Keys always get copied into the structures and not assigned!
Changing the value of the pointer given to the structure later on wont affect the contents of the structure!

# EXAMPLES
## VOID ARRAY
```c
#include <stdio.h>

#include "void_array.h"

int main() {
	struct void_array varray;
	varr_init(&varray, 10, sizeof(double), NULL);

	for (int i = 0; i < varray.capacity; i++) {
		double value = (i + 1) * 1.5;
		varr_add(&varray, &value);
	}


	printf("%f\n", varr_get(&varray, 6)); // 10.5

	// Values get pulled from the right at deletion
	varr_remove(&varray, 6);

	printf("%f\n", varr_get(&varray, 6)); // 12.0

	varr_free(&varray);

	return 0;
}

```

## VOID DICT
```c
#include <stdio.h>

#include "void_array.h"

int main() {
	struct void_dict vdict;
	vdict_init(&vdict, 25, sizeof(char) * 5, sizeof(double), NULL);


	double temp_c = 25.5;
	double rain_in_cm = 16;
	double uv_level = 3;

	vdict_add_pair(&vdict, "Temp", &temp_c);
	vdict_add_pair(&vdict, "Rain", &rain_in_cm);

	// The Keys must always be the given size. This could also be easily achived by creating an array.
	// const char uv_key[5] = "UV";
	// vdict_add_pair(&vdict, uv_key, &uv_level);
	vdict_add_pair(&vdict, "UV\0\0", &uv_level);


	printf("%f\n", vdict_get_value(&vdict, "Temp")); // 25.5

	vdict_free(&vdict);

	return 0;
}

```

## COMBINATION
```c
#include <stdio.h>

#include "void_array.h"
#include "void_dict.h"

int main() {
	struct void_dict vdict;
	vdict_init(&vdict, 10, sizeof(int), sizeof(struct void_array), varr_free);

	// i is 1 because keys can't be zero in a void dict
	for (int i = 1; i <= vdict.capacity; i++) {
		struct void_array sub_array;
		varr_init(&sub_array, 5, sizeof(double), NULL);
		for (int j = 0; j < varr.capacity; j++) {
			double value = (i + 1) * 1.5;
			varr_add(&sub_array, &value);
		}
		vdict_add_pair(&vdict, &i, &sub_array);

		// No Memory leakage because we told vdict how to free the non
		// standard values it contains (varr_free)
	}

	
	// Frees every value it contains and then itself
	vdict_free(&vdict);


	return 0;
}

```


# LICENSE - MIT
See `LICENSE` file in the root directory of this repository
