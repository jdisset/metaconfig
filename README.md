# A tiny configuration class generation tool
With automatic serialization and no runtime overhead.

## Example usage:

```
struct Cfg {
	struct SubCfg {
		METACONFIG_DECLARE(SubCfg, (int, a,), (float, b, 0.2f), (std::string, s, "default value"));
	};
	METACONFIG_DECLARE(Cfg, (std::vector<int>, vec,), (SubCfg, nestedCong,));
};
```

