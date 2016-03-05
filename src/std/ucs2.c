#include <hl.h>
#ifndef HL_NATIVE_WCHAR_FUN
#include <stdarg.h>

int ustrlen( const uchar *str ) {
	const uchar *p = str;
	while( *p ) p++;
	return (int)(p - str);
}

uchar *ustrdup( const uchar *str ) {
	int len = ustrlen(str);
	int size = (len + 1) << 1;
	uchar *d = (uchar*)malloc(size);
	memcpy(d,str,size);
	return d;
}

double utod( const uchar *str, uchar **end ) {
	char buf[31];
	char *bend;
	int i = 0;
	double v;
	while( *str == ' ' ) str++;
	while( i < 30 ) {
		int c = *str++;
		if( (c < '0' || c > '9') && c != '.' && c != 'e' && c != 'E' && c != '-' && c != '+' )
			break;
		buf[i++] = (char)c;
	}
	buf[i] = 0;
	v = strtod(buf,&bend);
	*end = (uchar*)(str - 1) + (bend - buf);
	return v;
}

int utoi( const uchar *str, uchar **end ) {
	char buf[17];
	char *bend;
	int i = 0;
	int v;
	while( *str == ' ' ) str++;
	while( i < 16 ) {
		int c = *str++;
		if( (c < '0' || c > '9') && c != '-' )
			break;
		buf[i++] = (char)c;
	}
	buf[i] = 0;
	v = strtol(buf,&bend,10);
	*end = (uchar*)(str - 1) + (bend - buf);
	return v;
}

int ucmp( const uchar *a, const uchar *b ) {
	while(true) {
		int d = (unsigned)*a - (unsigned)*b; 
		if( d ) return d;
		if( !*a ) return 0;
		a++;
		b++;
	}
}

int strtou( uchar *out, int out_size, const char *str ) {
	uchar *start = out;
	if( out_size <= 0 ) return -1;
	while( out_size-- ) {
		if( *str == 0 ) break;
		*out++ = *str++;
	}
	*out = 0;
	return (int)(out - start);
}

int uvsprintf( uchar *out, const uchar *fmt, va_list arglist ) {
	uchar *start = out;
	char cfmt[20];
	char tmp[32];
	uchar c;
	while(true) {
sprintf_loop:
		c = *fmt++;
		switch( c ) {
		case 0:
			*out = 0;
			return (int)(out - start);
		case '%':
			{
				int i = 0, size = 0;
				cfmt[i++] = '%';
				while( true ) {
					c = *fmt++;
					cfmt[i++] = (char)c;
					switch( c ) {
					case 'd':
						cfmt[i++] = 0;
						size = sprintf(tmp,cfmt,va_arg(arglist,int));
						goto sprintf_add;
					case 'f':
						cfmt[i++] = 0;
						size = sprintf(tmp,cfmt,va_arg(arglist,double)); // according to GCC warning, float is promoted to double in var_args
						goto sprintf_add;
					case 'g':
						cfmt[i++] = 0;
						size = sprintf(tmp,cfmt,va_arg(arglist,double));
						goto sprintf_add;
					case 'x':
					case 'X':
						cfmt[i++] = 0;
						if( cfmt[i-3] == 'l' )
							size = sprintf(tmp,cfmt,va_arg(arglist,void*));
						else
							size = sprintf(tmp,cfmt,va_arg(arglist,int));
						goto sprintf_add;
					case 's':
						if( i != 2 ) hl_fatal("Unsupported printf format"); // no support for precision qualifier
						{
							uchar *s = va_arg(arglist,uchar *);
							while( *s )
								*out++ = *s++;
							goto sprintf_loop;
						}
					case '.':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case 'l':
						break;
					default:
						hl_fatal("Unsupported printf format");
						break;
					}
				}
sprintf_add:
				// copy from c string to u string
				i = 0;
				while( i < size )
					*out++ = tmp[i++];
			}
			break;
		default:
			*out++ = c;
			break;
		}
	}
	return 0;
}

int usprintf( uchar *out, int out_size, const uchar *fmt, ... ) {
	va_list args;
	int ret;
	va_start(args, fmt);
	ret = uvsprintf(out, fmt, args);
	va_end(args);
	return ret;
}

// USE UTF-8 encoding
int utostr( char *out, int out_size, const uchar *str ) {
	char *start = out;
	char *end = out + out_size - 1; // final 0
	if( out_size <= 0 ) return 0;
	while( out < end ) {
		unsigned int c = *str++;
		if( c == 0 ) break;
		if( c < 0x80 )
			*out++ = c;
		else if( c < 0x800 ) {
			if( out + 2 > end ) break;
			*out++ = 0xC0|(c>>6);
			*out++ = 0x80|(c&63);
		} else {
			if( out + 3 > end ) break;
			*out++ = 0xE0|(c>>12);
			*out++ = 0x80|((c>>6)&63);
			*out++ = 0x80|(c&63);
		}
	}
	*out = 0;
	return (int)(out - start);
}

static char *utos( const uchar *s ) {
	int len = ustrlen(s);
	char *out = (char*)malloc(len + 1);
	if( utostr(out,len+1,s) < 0 )
		*out = 0;
	return out;
}

void uprintf( const uchar *fmt, const uchar *str ) {
	char *cfmt = utos(fmt);
	char *cstr = utos(str);
	printf(cfmt,cstr);
	free(cfmt);
	free(cstr);
}

#endif