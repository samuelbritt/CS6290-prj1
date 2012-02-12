/* wrapper to the actual main, so that we can unit test
 */
int main_(int, char const *);
int main(int argc, char const *argv[])
{
	return main_(argc, argv);
}
