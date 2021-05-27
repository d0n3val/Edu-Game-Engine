#ifndef _VISIT_VARIANT_H_
#define _VISIT_VARIANT_H_

// from  https://www.bfilipek.com/2018/09/visit-variants.html
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...)->overload<Ts...>;

#endif /* _VISIT_VARIANT */
