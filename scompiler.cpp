/*
 * 'scompiler.cpp' is a language S compiler that uses gcc
 * by Dario Sneidermanis
 *
 * Compilation: g++ -o scompiler scompiler.cpp
 *
 * Usage: ./scompiler [options] file
 * Options:
 *   -S         Compile only; leave assembler file
 *   -C         Do not delete intermediate C file
 *   -o <file>  Place the output file into <file>
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
using namespace std;

class program
{
	typedef pair< string, pair< char, int > > var;

	static map < string, int > functions;
	static vector< vector < int > > adj_list;

	static vector< program* > all_progs;
	static vector<int> rsorted_progs;
	static ofstream sal;

	static const string numbers;
	static const string letters;
	static const string space;

	static const string cio_type;

	static const string dirs;

	static bool debugging;

	vector < pair< int , string > > s_code;
	vector < string > c_code;

	set < string > labels;
	vector< pair< string, int > > extras;

	string name;
	int adj_pos;
	bool main;
	bool inlining;

	int max_x, max_z;

	public:

	program( string _name, bool main = true ): main(main)
	{
		if( main )
			all_progs.resize( 1, this );

		name = _name;

		int i = name.find_last_of("/");
		name.erase( 0, ( i == string::npos ) ? 0 : i+1 );
		i = name.find_first_of(".");
		name.erase( ( i == string::npos ) ? name.size() : i );

		if( functions.find( name ) != functions.end() )
			error( "Code 100, please file a bug report" );

		adj_pos = adj_list.size();

		functions.insert( make_pair( name, adj_pos ) );

		adj_list.push_back( vector<int>() );

		max_x = max_z = 0;
		inlining = false;

		read( _name );
		pre_proc();
		translate();
		post_proc();
	}

	~program()
	{
		if( main )
			for( int i = program::all_progs.size()-1; i>0; i-- )
				delete program::all_progs[i];
	}

	void write()
	{
		if( main )
		{
			sal.open( (name+".c").c_str() );

			if(!sal)
				error("while opening destination file");

			sal << "/*\n * Generated automatically with 'scompiler' by Dario Sneidermanis\n */\n\n";
			sal << "#include <stdio.h>\n";
			sal << "#include <string.h>\n";
			sal << "#include <stdarg.h>\n";
			sal << "#include <assert.h>\n";
			sal << "typedef unsigned long natural;\n";
			sal << "\nva_list argptr;\n";
			sal << "\n#ifndef __inline__\n\t#define __inline__\n#endif\n";

			sal << "\n#define header( max_x, max_z ) \\\n";
			sal << "\tnatural X[max_x], Y = 0, Z[max_z], i;\\\n";
			sal << "\tmemset( X, '\\0', sizeof(X) );\\\n";
			sal << "\tmemset( Z, '\\0', sizeof(Z) );\\\n";
			sal << "\tva_start( argptr, n );\\\n";
			sal << "\tfor( i = 1; i < max_x && i <= n; i++ )\\\n";
			sal << "\t\tX[i] = va_arg( argptr, natural );\\\n";
			sal << "\tva_end( argptr )" << ((debugging)?";\\":"") << "\n" ;
			if( debugging )
				sal << "\tprintf(\"In "+name+"\\n\")\n";

			topo_sort(0);
			rsorted_progs.pop_back();

			vector<bool> writen( rsorted_progs.size() + 1, false );

			for( vector<int>::iterator it = rsorted_progs.begin(); it != rsorted_progs.end(); ++it )
				if( !writen[*it] )
				{
					writen[*it] = true;
					all_progs[*it]->write();
				}
		}

		sal << endl;

		for( vector<string>::iterator it = c_code.begin(); it != c_code.end(); ++it )
			sal << (*it);

		if( main )
			sal.close();
	}

	private:

	program();

	program operator=( program& p );

	//******* AUXILIARY FUNCTIONS ********

	int to_int( string s )
	{
		istringstream is(s+" -1");
		int i;
		is >> i;
		return i;
	}

	string to_str( int n )
	{
		ostringstream os;
		os << n;
		return os.str();
	}

	void topo_sort( int i )
	{
		static vector< bool > touched( adj_list.size(), false );

		if( touched[i] )
		{
			sal << "\nnatural __"+all_progs[i]->name+"( natural n, ... );\n";
			warning( "undirect recursion" );
			return;
		}
		touched[i] = true;

		for( vector<int>::iterator it = adj_list[i].begin(); it != adj_list[i].end(); ++it )
			topo_sort(*it);

		rsorted_progs.push_back( i );
		touched[i] = false;
	}

	void error( string msg )
	{
		cout << name << ".s: error: " << msg << endl;
		exit(0);
	}

	void error( int lineno, string msg )
	{
		cout << name << ".s: " << lineno << ": error: " << msg << endl;
		exit(0);
	}

	void warning( string msg )
	{
		cout << name << ".s: warning: " << msg << endl;
	}

	void warning( int lineno, string msg )
	{
		cout << name << ".s: " << lineno << ": warning: " << msg << endl;
	}

	//********** MAIN FUNCTIONS *********

	void read( string _name )
	{
		ifstream ent( _name.c_str() );
		int last = 0;

		if( !ent )
		{
			int i = _name.find_last_of("/");
			_name.erase( 0, ( i == string::npos ) ? 0 : i+1 );
			if( _name.find(".") == string::npos )
				_name += ".s";

			for( int i = 0; i < dirs.size(); i++ )
				if( dirs[i] == ';' )
				{
					ent.close();
					ent.clear();
					ent.open( ( dirs.substr( last, i-last ) + _name ).c_str() );
					if(ent)
						break;
					last = i+1;
				}
			if( !ent )
				error( ": no such file" );
		}

		string s;
		int lineno = 0,i;

		while ( getline( ent, s ) )
		{
			lineno++;

			if( ( i = s.find_first_not_of( space ) ) != string::npos )
			{
				s.erase( 0, i );
				s_code.push_back( make_pair( lineno, s ) );
			}
		}
	}

	void add_label( int lineno, int fline, string label )
	{
		int start = min( label.find_first_not_of( space ), label.size() );
		int end   = min( label.find_last_not_of( space ), label.size() );

		label = label.substr( start, end-start+1 );

		if( label.empty() )
			error( lineno, "empty label" );

		if( label.find_first_of( space ) != string::npos )
			error( lineno, "illegal label name: space in label" );

		if( label.find_last_not_of( numbers ) != 0 )
			error( lineno, "illegal label name: must be a letter followed by a *number*" );

		if( label.find_first_of( letters ) != 0 )
			error( lineno, "illegal label name: must be a *letter* followed by a number" );

		labels.insert( label );

		extras.push_back( make_pair( label+":", fline ) );
	}

	void read_label( int lineno, int fline, string &ln )
	{
		read_str( lineno, ln, "[" );

		int end = ln.find_first_of("]");

		if( end == string::npos )
			error( lineno, "']' expected" );

		add_label( lineno, fline, ln.substr( 0, end ) );

		ln.erase( 0, end+1 );
	}

	void read_extra( int lineno, int fline, string &ln )
	{
		read_str( lineno, ln, "{" );

		var v=read_var( ln );

		if( !v.second.first )
		{
			if( v.first == "debug") debugging = true;
			else
			if( v.first == "inline") inlining = true;
			else
			if( v.first == "wait" ) extras.push_back( make_pair( "getchar();", fline ) );
			else
				error( lineno, "'"+v.first+"' is not a variable" );
		}else
			extras.push_back(
				make_pair( "printf(\""+v.first+" = %"+cio_type+"\\n\","+v.first+");", fline ) );

		read_str( lineno, ln, "}" );
	}

	void pre_proc()
	{
		int fline = -1;
		vector<pair<int,string> >::iterator line;

		for(  line = s_code.begin(); line != s_code.end(); ++line )
		{
			string &ln = line->second;
			int lineno = line->first;

			fline++;

			ln.erase( min( ln.find("//"), ln.size() ) );

			while( true )
			{
				int i = ln.find_first_not_of( space );

				if( i == string::npos )
				{
					fline--;
					line = s_code.erase( line ) - 1;
					break;
				}

				if( ln[i] != '[' && ln[i] != '{' )
					break;

				if( ln[i] == '[' )
					read_label( lineno, fline, ln );

				if( ln[i] == '{' )
					read_extra( lineno, fline, ln );
			}
		}
	}

	void post_proc()
	{
		vector < string > header;
		string s_max_x = to_str( max_x+1 );
		string s_max_z = to_str( max_z+1 );

		if( inlining )
			header.push_back( "__inline__" );
		header.push_back( "natural __"+name+"( natural n, ... ) {" );
		header.push_back( "\theader( "+s_max_x+", "+s_max_z+");" );

		vector < pair < string , int > >::reverse_iterator it;

		for( it = extras.rbegin(); it != extras.rend(); ++it )
		{
			c_code[ it->second ] = it->first + c_code[ it->second ];
		}

		c_code.insert( c_code.begin(), header.begin(), header.end() );
		c_code.push_back( "\treturn Y;" );
		c_code.push_back( "}" );

		if(main)
		{
			c_code.push_back( "\nint main( int argc, char** argv) {" );
			c_code.push_back( "\tnatural X["+s_max_x+"], i;" );
			c_code.push_back( "\tchar c;" );
			c_code.push_back( "\tmemset( X, '\\0', sizeof(X) );" );
			c_code.push_back( "\tfor( i=1; i<"+s_max_x+" && i<(natural)argc; i++ ) {" );
			c_code.push_back( "\t\tassert( !sscanf( argv[i], \" -%c\", &c ) );" );
			c_code.push_back(
				"\t\tassert( sscanf( argv[i], \" %"+cio_type+"%c\", &X[i], &c ) == 1 );" );
			c_code.push_back( "\t}" );
			c_code.push_back( "\tprintf( \"%"+cio_type+"\\n\", __"+name+"( "+to_str(max_x) );
			for( int i = 1; i <= max_x; i++ )
				c_code.push_back( "\t\t\t\t\t\t\t, X["+to_str(i)+"]" );
			c_code.push_back( "\t\t\t\t\t\t\t) );" );
			c_code.push_back( "\treturn 0;\n}" );
		}

		for( vector<string>::iterator i=c_code.begin(); i!=c_code.end() ; ++i )
			i->push_back('\n');
	}

	var read_var( string& s, int pos = 0 )
	{
		var v;
		int begin = min( s.find_first_not_of( space, pos ), s.size() );
		int end = min( s.find_first_not_of( letters+numbers+"_/", begin ), s.size() );

		v.first = s.substr( begin, end-begin );

		s.erase( 0, end );

		char L = toupper(v.first[0]);
		if( ( L != 'X' && L != 'Y' && L !='Z') ||
			( L == 'Y' && v.first.size() != 1 ) ||
			( L != 'Y' && v.first.size() == 1 ) ||
			( L != 'Y' && v.first.find_first_not_of( numbers, 1 ) != string::npos ) )
		{
			v.second = make_pair(0,0);
			return v;
		}

		v.first[0] = toupper( v.first[0] );
		v.second.first = v.first[0];
		v.second.second = to_int( v.first.substr(1) );

		if( v.second.first != 'Y' )
		{
			v.first.insert( 1,"[" );
			v.first.push_back(']');

			if( v.second.first == 'X' )
				max_x = max( max_x, v.second.second );
			else
				max_z = max( max_z, v.second.second );
		}

		return v;
	}

	void read_str( int lineno, string &where, string which, int pos = 0 )
	{
		int k = where.find_first_not_of( space, pos );

		if( k != where.find( which, pos ) || k == string::npos )
			error( lineno, "'" + which + "' expected" );

		where.erase( 0, k + which.size() );
	}

	string read_arg( int lineno, string &ln )
	{
		int i = ln.find_first_not_of( space );

		ln.erase( 0, i );

		if( ln[0] == ')' )
			return "";

		if( ( ln[0] == '(' || ln[0] == ',' ) && ln.find_first_not_of( space, 1 ) == ')' )
			return "";

		var V = read_var( ln, 1 );

		if( V.second.first )
			return V.first;

		if( V.first.find_first_not_of( numbers ) == string::npos )
			return V.first;

		string sol = read_macro( lineno, ln, V.first );

		if( ln.find_first_not_of( space ) < ln.find_first_of(",)") )
			error( lineno, "comma expected before another parameter" );

		return sol;
	}

	void add_macro( int lineno, string name )
	{
		if( name.size() == 0 || string( 1, name[0]).find_first_of( numbers ) == 0 )
			error( lineno, "illegal macro name: '"+name+"'" );

		map< string, int >::iterator it = functions.find( name );

		if( it != functions.end() )
		{
			if( it->second != adj_pos )
				adj_list[ adj_pos ].push_back( it->second );
		}else
		{
			adj_list[ adj_pos ].push_back( adj_list.size() );

			int pos = all_progs.size();
			all_progs.push_back( NULL );
			all_progs[ pos ] = new program( name, false );
		}
	}

	string read_macro( int lineno, string &ln, string name )
	{

		add_macro( lineno, name );

		int i = name.find_last_of("/");
		name.erase( 0, ( i == string::npos ) ? 0 : i+1 );

		int begin = ln.find_first_not_of(space);

		ln.erase( 0, begin );

		vector<string> args;

		if( ln[0] == '(' )
		{
			do{
				args.push_back( read_arg( lineno, ln ) );
			}while( args.back().size() );

			ln.erase( 0, 1 );
		}
		else
		if( ln[0] != ',' )
			error( lineno, " '(' or ',' expected" );

		args.pop_back();

		string sol = "__" + name + "( "+ to_str( args.size() ) +", ";

		for( vector<string>::iterator i = args.begin(); i != args.end(); ++i )
			sol += (*i) + ", ";

		return sol.erase( sol.size()-2 ) + " )";
	}

	void read_assignment( int lineno, string &ln, var &V )
	{
		read_str( lineno, ln, "<-" );

		var W = read_var(ln);

		if( V == W )
		{
			int k = ln.find_first_not_of( space );

			if( k == string::npos || ( ln[k] != '+' && ln[k] != '-' ) )
				error( lineno, "+/- expected" );

			bool plus = ln[k]=='+';

			read_str( lineno, ln, "1", k+1 );

			if( plus )
				c_code.push_back( "\t" + V.first + "++;" );
			else
				c_code.push_back( "\tif( " + V.first + " ) " + V.first + "--;" );
		}
		else
		{
			if( W.second.first )
				error( lineno, "macro expected" );

			c_code.push_back( "\t" + V.first + " = " + read_macro( lineno , ln, W.first ) + ";" );
		}

		if( ln.find_first_not_of( space ) != string::npos )
			error( lineno, "EOL expected" );
	}

	void read_if( int lineno, string &ln )
	{
		var V = read_var( ln );

		if( !V.second.first )
		{
			V.first = read_macro( lineno, ln, V.first );
		}
		read_str( lineno, ln, "!=" );
		read_str( lineno, ln, "0" );
		read_str( lineno, ln, "goto" );

		int start = min( ln.find_first_not_of(space), ln.size() );
		int end   = min( ln.find_last_not_of(space), ln.size() );

		if( ln[start] != '[' || ln[end] != ']' )
			error( lineno, "[label] expected" );

		ln = ln.substr( start+1, end-start-1 );

		if( start == string::npos || end == string::npos || !ln.size() )
			error( lineno, "[label] expected" );

		set < string >::iterator i = labels.find(ln);

		if( i != labels.end() )
			c_code.push_back("\tif( "+V.first+" ) goto "+ln+";");
		else{
			if( ln != "E" )
				warning( lineno, "'"+ln+"' there is no such label, forcing exit" );

			c_code.push_back("\tif( "+V.first+" ) return Y;");
		}
	}

	void translate()
	{
		for( vector<pair<int,string> >::iterator line = s_code.begin() ;
													line != s_code.end() ; ++line )
		{
			string ln = line->second;
			int lineno = line->first;

			var V = read_var(ln);

			if( V.second.first )

				read_assignment( lineno, ln, V );

			else{

				if ( V.first != "if" && V.first != "IF" && V.first != "iF" && V.first != "If" )
					error( lineno, "lines must begin with a variable or an if statement" );

				read_if( lineno, ln );
			}
		}
	}

};

map < string, int >	program::functions;
vector<vector<int> >program::adj_list;

vector < program* >	program::all_progs;
vector < int >		program::rsorted_progs;
ofstream			program::sal;

const string program::numbers("1234567890");
const string program::letters("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM");
const string program::space(" \t\r\n");

const string program::cio_type("lu");

const string program::dirs(";s_source/;s_source/basic/;");

bool program::debugging(false);


int main( int argc, char** argv )
{
	ios::sync_with_stdio(false);

	if( argc < 2 )
	{
		cout << "Usage: ./scompiler [options] file" << endl;
		cout << "Options:" << endl;
		cout << "  -S\t\tCompile only; leave assembler file" << endl;
		cout << "  -C\t\tDo not delete intermediate C file" << endl;
		cout << "  -o <file>\tPlace the output file into <file>" << endl;
		return 0;
	}

	string name;
	string oname;
	string comp_options("gcc -ansi -pedantic -Wall -Wextra -O3 ");
	bool erase_c = true;
	bool jasm = false;

	for( int n = 1; n < argc; n++ )
	{
		if( argv[n][0] == '-' )
			switch( argv[n][1] )
			{
				case 'C':
					erase_c = false;
					break;
				case 'S':
					jasm = true;
					comp_options+="-S ";
					break;
				case 'o':
					if( n+1 < argc && oname == "" ){
						oname = oname+"-o "+argv[++n];
						break;
					}
				default:
					cout << "error: unrecognized command line option '" << argv[n] << "'\n";
					return 0;
			}
		else{
			if( name != "" )
			{
				cout << "error: you can compile just one file at a time\n";
				return 0;
			}
			name = argv[n];
		}
	}

	if( name == "" )
	{
		cout << "error: no input file\n";
		return 0;
	}

	program( name ).write();

	int i = name.find_last_of("/");
	name.erase( 0, ( i == string::npos )? 0 : i+1 );
	i = name.find_first_of(".");
	name.erase( ( i == string::npos )? name.size() : i );

	if( oname == "" )
		oname = "-o "+name+((jasm)?".asm":"");

	if( system( ( comp_options+oname+' '+name+".c" ).c_str() ) )
		cout << "Error while compiling\n";

	if( erase_c && system( ( "rm -f "+name+".c" ).c_str() ) );

	return 0;
}
