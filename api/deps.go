// This file exists to ensure that go.mod dependencies are properly resolved by Gazelle.
// The blank imports below force the specific versions declared in go.mod to be used.
//
// This is particularly important for google.golang.org/protobuf v1.36.10 which is required
// by the protovalidate generated code that uses MessageFieldStringOf.

//go:build tools
// +build tools

package api

import (
	_ "google.golang.org/protobuf/runtime/protoimpl"
)
