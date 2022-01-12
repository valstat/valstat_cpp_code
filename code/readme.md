# Very important macro use case

> This was started on the WG21 mailing list, circa 2021 Sep 

Valstat first stage of deciphering is not based on any type. It is based on the empty/not empty state of the value and status fields. Thus we can create and use two very handy macros.  Here is the imaginary code using them:
```cpp
// C++ 
// connect to the contacts server
// return the city associated with the current user
// each function returns **different** valstat structure
valstat<std::string> FindUsersCity() noexcept
{
	vscall(contacts, GetOrOpenContactsServerConnection());
	vscall(uid, contacts.value->GetUserId());
	vscall(geo, GetOrOpenGeoServerConnection());
	vscall(uloc, geo.value->GetLocation(&*uid.value));
	vscall(cityname, (uloc.value)->GetCityName());
	// all the call's above do return_on_no_value
	// thus the value must exist here 
	return { { *cityname.value } , {} };
}
```
That is very clean code. Above code without valstat protocol implementation will be much more complex. That might be the proverbial exercise for the reader.

Here are the macros used above. 
```cpp
// C++

#define vsreturn_on_no_value(VS) if ( ! VS.value ) return {{}, VS.status}

// call the valstat returning function
// F can be also another matching valstat intance
// save the result as `VS`
#define vscall( VS, F ) auto VS = F ;	vsreturn_on_no_value(VS)
```
`vscall` actually returns on error and error is "value field is empty"
note the absolute key and core of the trick here: we do not look into the value, thus we do not need to define what does it mean "empty" for each and every
type, value type might be instead we just check if field potentially holding it is empty notice we can return as we do because we know valstat is return type and valstat is an record of two fields.


