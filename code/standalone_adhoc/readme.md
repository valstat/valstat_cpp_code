# Stand Alone Ad Hoc 

Experiment: Let's use valstat just as a spec. An architecture with no implementation mandated whatsoever.

How would such a code look like? Well it is in here.

- Does it add to the code complexity? It does.
- is it fit for the purpose? It is.
  - all is returned and all the errors are properly signalled
    - no c++ exception
    - no std::system_error
    - no special return types a la std::expect
- valstat is leading to the code requiring precision and discipline
  - it actually enforces it onto you

---
&copy; 2022 by dbj at dbj dot org
https://dbj.org/license_dbj